// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>
#include <utility>
#include "audio_core/dsp_interface.h"
#include "audio_core/hle/hle.h"
#include "audio_core/lle/lle.h"
#include "common/logging/log.h"
#include "common/texture.h"
#include "core/arm/arm_interface.h"
#ifdef ARCHITECTURE_x86_64
#include "core/arm/dynarmic/arm_dynarmic.h"
#endif
#include "core/arm/dyncom/arm_dyncom.h"
#include "core/cheats/cheats.h"
#include "core/core.h"
#include "core/core_timing.h"
#include "core/custom_tex_cache.h"
#include "core/dumping/backend.h"
#ifdef ENABLE_FFMPEG_VIDEO_DUMPER
#include "core/dumping/ffmpeg_backend.h"
#endif
#include "core/gdbstub/gdbstub.h"
#include "core/hle/kernel/client_port.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/process.h"
#include "core/hle/kernel/thread.h"
#include "core/hle/service/fs/archive.h"
#include "core/hle/service/service.h"
#include "core/hle/service/sm/sm.h"
#include "core/hw/hw.h"
#include "core/loader/loader.h"
#include "core/movie.h"
#include "core/rpc/rpc_server.h"
#include "core/settings.h"
#include "network/network.h"
#include "video_core/video_core.h"

namespace Core {

/*static*/ System System::s_instance;

System::ResultStatus System::RunLoop(bool tight_loop) {
    status = ResultStatus::Success;
    if (!cpu_core) {
        return ResultStatus::ErrorNotInitialized;
    }

    if (GDBStub::IsServerEnabled()) {
        GDBStub::HandlePacket();

        // If the loop is halted and we want to step, use a tiny (1) number of instructions to
        // execute. Otherwise, get out of the loop function.
        if (GDBStub::GetCpuHaltFlag()) {
            if (GDBStub::GetCpuStepFlag()) {
                tight_loop = false;
            } else {
                return ResultStatus::Success;
            }
        }
    }

    // If we don't have a currently active thread then don't execute instructions,
    // instead advance to the next event and try to yield to the next thread
    if (kernel->GetThreadManager().GetCurrentThread() == nullptr) {
        LOG_TRACE(Core_ARM11, "Idling");
        timing->Idle();
        timing->Advance();
        PrepareReschedule();
    } else {
        timing->Advance();
        if (tight_loop) {
            cpu_core->Run();
        } else {
            cpu_core->Step();
        }
    }

    if (GDBStub::IsServerEnabled()) {
        GDBStub::SetCpuStepFlag(false);
    }

    HW::Update();
    Reschedule();

    if (reset_requested.exchange(false)) {
        Reset();
    } else if (shutdown_requested.exchange(false)) {
        return ResultStatus::ShutdownRequested;
    }

    return status;
}

System::ResultStatus System::SingleStep() {
    return RunLoop(false);
}

System::ResultStatus System::Load(Frontend::EmuWindow& emu_window, const std::string& filepath) {
    app_loader = Loader::GetLoader(filepath);
    if (!app_loader) {
        LOG_CRITICAL(Core, "Failed to obtain loader for {}!", filepath);
        return ResultStatus::ErrorGetLoader;
    }
    std::pair<std::optional<u32>, Loader::ResultStatus> system_mode =
        app_loader->LoadKernelSystemMode();

    if (system_mode.second != Loader::ResultStatus::Success) {
        LOG_CRITICAL(Core, "Failed to determine system mode (Error {})!",
                     static_cast<int>(system_mode.second));

        switch (system_mode.second) {
        case Loader::ResultStatus::ErrorEncrypted:
            return ResultStatus::ErrorLoader_ErrorEncrypted;
        case Loader::ResultStatus::ErrorInvalidFormat:
            return ResultStatus::ErrorLoader_ErrorInvalidFormat;
        default:
            return ResultStatus::ErrorSystemMode;
        }
    }

    ASSERT(system_mode.first);
    ResultStatus init_result{Init(emu_window, *system_mode.first)};
    if (init_result != ResultStatus::Success) {
        LOG_CRITICAL(Core, "Failed to initialize system (Error {})!",
                     static_cast<u32>(init_result));
        System::Shutdown();
        return init_result;
    }

    telemetry_session->AddInitialInfo(*app_loader);
    std::shared_ptr<Kernel::Process> process;
    const Loader::ResultStatus load_result{app_loader->Load(process)};
    kernel->SetCurrentProcess(process);
    if (Loader::ResultStatus::Success != load_result) {
        LOG_CRITICAL(Core, "Failed to load ROM (Error {})!", static_cast<u32>(load_result));
        System::Shutdown();

        switch (load_result) {
        case Loader::ResultStatus::ErrorEncrypted:
            return ResultStatus::ErrorLoader_ErrorEncrypted;
        case Loader::ResultStatus::ErrorInvalidFormat:
            return ResultStatus::ErrorLoader_ErrorInvalidFormat;
        default:
            return ResultStatus::ErrorLoader;
        }
    }
    cheat_engine = std::make_unique<Cheats::CheatEngine>(*this);

    custom_tex_cache = std::make_unique<Core::CustomTexCache>();
    const std::string_view preload_dir = Settings::values.preload_textures_dir;
    const std::string_view load_dir = FileUtil::GetUserPath(FileUtil::UserPath::LoadDir);

    const std::string sanitized_preload_dir = FileUtil::SanitizePath(preload_dir);
    const std::string sanitized_load_dir = FileUtil::SanitizePath(load_dir);

    const u64 program_id = Kernel().GetCurrentProcess()->codeset->program_id;

    if (sanitized_load_dir != sanitized_preload_dir && Settings::values.custom_textures &&
        Settings::values.preload_textures) {
        LOG_INFO(Core,
                 "Using alternative strategy for preloading textures. Texture preload base "
                 "directory is {}",
                 sanitized_preload_dir);

        const std::string preload_path =
            fmt::format("{}/textures/{:016X}/", sanitized_preload_dir, program_id);
        FileUtil::CreateFullPath(preload_path);
        custom_tex_cache->FindCustomTextures(preload_path);

        custom_tex_cache->PreloadTextures(*GetImageInterface());

        FileUtil::CreateFullPath(
            fmt::format("{}/textures/{:016X}/", sanitized_load_dir, program_id));
        custom_tex_cache->FindCustomTextures(program_id);
    } else {
        if (Settings::values.custom_textures) {
            FileUtil::CreateFullPath(
                fmt::format("{}/textures/{:016X}/", sanitized_load_dir, program_id));
            custom_tex_cache->FindCustomTextures(program_id);
        }
        if (Settings::values.preload_textures) {
            custom_tex_cache->PreloadTextures(*GetImageInterface());
        }
    }

    u64 title_id{0};
    if (app_loader->ReadProgramId(title_id) != Loader::ResultStatus::Success) {
        LOG_ERROR(Core, "Failed to find title id for ROM (Error {})",
                  static_cast<u32>(load_result));
    }
    perf_stats = std::make_unique<PerfStats>(title_id);

    status = ResultStatus::Success;
    m_emu_window = &emu_window;
    m_filepath = filepath;

    // Reset counters and set time origin to current frame
    GetAndResetPerfStats();
    perf_stats->BeginSystemFrame();
    return status;
}

void System::PrepareReschedule() {
    cpu_core->PrepareReschedule();
    reschedule_pending = true;
}

PerfStats::Results System::GetAndResetPerfStats() {
    return perf_stats->GetAndResetStats(timing->GetGlobalTimeUs());
}

void System::Reschedule() {
    if (!reschedule_pending) {
        return;
    }

    reschedule_pending = false;
    kernel->GetThreadManager().Reschedule();
}

System::ResultStatus System::Init(Frontend::EmuWindow& emu_window, u32 system_mode) {
    LOG_DEBUG(HW_Memory, "initialized OK");

    memory = std::make_unique<Memory::MemorySystem>();

    timing = std::make_unique<Timing>();

    kernel = std::make_unique<Kernel::KernelSystem>(*memory, *timing,
                                                    [this] { PrepareReschedule(); }, system_mode);

    if (Settings::values.use_cpu_jit) {
#ifdef ARCHITECTURE_x86_64
        cpu_core = std::make_shared<ARM_Dynarmic>(this, *memory, USER32MODE);
#else
        cpu_core = std::make_shared<ARM_DynCom>(this, *memory, USER32MODE);
        LOG_WARNING(Core, "CPU JIT requested, but Dynarmic not available");
#endif
    } else {
        cpu_core = std::make_shared<ARM_DynCom>(this, *memory, USER32MODE);
    }

    kernel->SetCPU(cpu_core);

    if (Settings::values.enable_dsp_lle) {
        dsp_core = std::make_unique<AudioCore::DspLle>(*memory,
                                                       Settings::values.enable_dsp_lle_multithread);
    } else {
        dsp_core = std::make_unique<AudioCore::DspHle>(*memory);
    }

    memory->SetDSP(*dsp_core);

    dsp_core->SetSink(Settings::values.sink_id, Settings::values.audio_device_id);
    dsp_core->EnableStretching(Settings::values.enable_audio_stretching);

    telemetry_session = std::make_unique<Core::TelemetrySession>();

    rpc_server = std::make_unique<RPC::RPCServer>();

    service_manager = std::make_shared<Service::SM::ServiceManager>(*this);
    archive_manager = std::make_unique<Service::FS::ArchiveManager>(*this);

    HW::Init(*memory);
    Service::Init(*this);
    GDBStub::Init();

    VideoCore::ResultStatus result = VideoCore::Init(emu_window, *memory);
    if (result != VideoCore::ResultStatus::Success) {
        switch (result) {
        case VideoCore::ResultStatus::ErrorGenericDrivers:
            return ResultStatus::ErrorVideoCore_ErrorGenericDrivers;
        case VideoCore::ResultStatus::ErrorBelowGL33:
            return ResultStatus::ErrorVideoCore_ErrorBelowGL33;
        default:
            return ResultStatus::ErrorVideoCore;
        }
    }

#ifdef ENABLE_FFMPEG_VIDEO_DUMPER
    video_dumper = std::make_unique<VideoDumper::FFmpegBackend>();
#else
    video_dumper = std::make_unique<VideoDumper::NullBackend>();
#endif

    LOG_DEBUG(Core, "Initialized OK");

    return ResultStatus::Success;
}

RendererBase& System::Renderer() {
    return *VideoCore::g_renderer;
}

Service::SM::ServiceManager& System::ServiceManager() {
    return *service_manager;
}

const Service::SM::ServiceManager& System::ServiceManager() const {
    return *service_manager;
}

Service::FS::ArchiveManager& System::ArchiveManager() {
    return *archive_manager;
}

const Service::FS::ArchiveManager& System::ArchiveManager() const {
    return *archive_manager;
}

Kernel::KernelSystem& System::Kernel() {
    return *kernel;
}

const Kernel::KernelSystem& System::Kernel() const {
    return *kernel;
}

Timing& System::CoreTiming() {
    return *timing;
}

const Timing& System::CoreTiming() const {
    return *timing;
}

Memory::MemorySystem& System::Memory() {
    return *memory;
}

const Memory::MemorySystem& System::Memory() const {
    return *memory;
}

Cheats::CheatEngine& System::CheatEngine() {
    return *cheat_engine;
}

const Cheats::CheatEngine& System::CheatEngine() const {
    return *cheat_engine;
}

Core::CustomTexCache& System::CustomTexCache() {
    return *custom_tex_cache;
}

const Core::CustomTexCache& System::CustomTexCache() const {
    return *custom_tex_cache;
}

VideoDumper::Backend& System::VideoDumper() {
    return *video_dumper;
}

const VideoDumper::Backend& System::VideoDumper() const {
    return *video_dumper;
}

void System::RegisterMiiSelector(std::shared_ptr<Frontend::MiiSelector> mii_selector) {
    registered_mii_selector = std::move(mii_selector);
}

void System::RegisterSoftwareKeyboard(std::shared_ptr<Frontend::SoftwareKeyboard> swkbd) {
    registered_swkbd = std::move(swkbd);
}

void System::RegisterImageInterface(std::shared_ptr<Frontend::ImageInterface> image_interface) {
    registered_image_interface = std::move(image_interface);
}

void System::Shutdown() {
    // Log last frame performance stats
    const auto perf_results = GetAndResetPerfStats();
    telemetry_session->AddField(Telemetry::FieldType::Performance, "Shutdown_EmulationSpeed",
                                perf_results.emulation_speed * 100.0);
    telemetry_session->AddField(Telemetry::FieldType::Performance, "Shutdown_Framerate",
                                perf_results.game_fps);
    telemetry_session->AddField(Telemetry::FieldType::Performance, "Shutdown_Frametime",
                                perf_results.frametime * 1000.0);
    telemetry_session->AddField(Telemetry::FieldType::Performance, "Mean_Frametime_MS",
                                perf_stats->GetMeanFrametime());

    // Shutdown emulation session
    GDBStub::Shutdown();
    VideoCore::Shutdown();
    HW::Shutdown();
    telemetry_session.reset();
    perf_stats.reset();
    rpc_server.reset();
    cheat_engine.reset();
    archive_manager.reset();
    service_manager.reset();
    dsp_core.reset();
    cpu_core.reset();
    kernel.reset();
    timing.reset();
    app_loader.reset();

    if (video_dumper->IsDumping()) {
        video_dumper->StopDumping();
    }

    if (auto room_member = Network::GetRoomMember().lock()) {
        Network::GameInfo game_info{};
        room_member->SendGameInfo(game_info);
    }

    LOG_DEBUG(Core, "Shutdown OK");
}

void System::Reset() {
    // This is NOT a proper reset, but a temporary workaround by shutting down the system and
    // reloading.
    // TODO: Properly implement the reset

    Shutdown();
    // Reload the system with the same setting
    Load(*m_emu_window, m_filepath);
}

} // namespace Core
