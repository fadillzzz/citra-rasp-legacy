// Copyright 2019 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QColorDialog>
#include <QFileDialog>
#include "citra_qt/configuration/configure_enhancements.h"
#include "core/core.h"
#include "core/settings.h"
#include "ui_configure_enhancements.h"
#include "video_core/renderer_opengl/post_processing_opengl.h"

ConfigureEnhancements::ConfigureEnhancements(QWidget* parent)
    : QWidget(parent), ui(new Ui::ConfigureEnhancements) {
    ui->setupUi(this);
    SetConfiguration();

    ui->AddTicks->setEnabled(Settings::values.FMV_hack);
    connect(ui->FMV_hack, &QCheckBox::toggled, ui->AddTicks, &QSpinBox::setEnabled);

    ui->layoutBox->setEnabled(!Settings::values.custom_layout);

    ui->resolution_factor_combobox->setEnabled(Settings::values.use_hw_renderer);

    connect(ui->render_3d_combobox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            [this](int currentIndex) {
                updateShaders(static_cast<Settings::StereoRenderOption>(currentIndex) ==
                              Settings::StereoRenderOption::Anaglyph);
            });

    connect(ui->bg_button, &QPushButton::clicked, this, [this] {
        const QColor new_bg_color = QColorDialog::getColor(bg_color);
        if (!new_bg_color.isValid()) {
            return;
        }
        bg_color = new_bg_color;
        QPixmap pixmap(ui->bg_button->size());
        pixmap.fill(bg_color);
        const QIcon color_icon(pixmap);
        ui->bg_button->setIcon(color_icon);
    });

    ui->toggle_preload_textures->setEnabled(ui->toggle_custom_textures->isChecked());
    connect(ui->toggle_custom_textures, &QCheckBox::toggled, this, [this] {
        ui->toggle_preload_textures->setEnabled(ui->toggle_custom_textures->isChecked());
        if (!ui->toggle_preload_textures->isEnabled())
            ui->toggle_preload_textures->setChecked(false);
    });

    connect(ui->change_preload_dir, &QPushButton::clicked, this, [this]() {
        const QString dir_path = QFileDialog::getExistingDirectory(
            this, tr("Select Preload Directory"),
            QString::fromStdString(Settings::values.preload_textures_dir),
            QFileDialog::ShowDirsOnly);
        if (!dir_path.isEmpty()) {
            Settings::values.preload_textures_dir = dir_path.toStdString();
            SetConfiguration();
        }
    });
}

void ConfigureEnhancements::SetConfiguration() {
    ui->resolution_factor_combobox->setCurrentIndex(Settings::values.resolution_factor);
    ui->render_3d_combobox->setCurrentIndex(static_cast<int>(Settings::values.render_3d));
    ui->factor_3d->setValue(Settings::values.factor_3d);
    updateShaders(Settings::values.render_3d == Settings::StereoRenderOption::Anaglyph);
    ui->toggle_linear_filter->setChecked(Settings::values.filter_mode);
    ui->layout_combobox->setCurrentIndex(static_cast<int>(Settings::values.layout_option));
    ui->swap_screen->setChecked(Settings::values.swap_screen);
    ui->toggle_dump_textures->setChecked(Settings::values.dump_textures);
    ui->toggle_custom_textures->setChecked(Settings::values.custom_textures);
    ui->toggle_preload_textures->setChecked(Settings::values.preload_textures);
    QString preload_path = QString::fromStdString(Settings::values.preload_textures_dir);
    ui->preload_dir_path->setText(preload_path);
    bg_color = QColor::fromRgbF(Settings::values.bg_red, Settings::values.bg_green,
                                Settings::values.bg_blue);
    QPixmap pixmap(ui->bg_button->size());
    pixmap.fill(bg_color);
    const QIcon color_icon(pixmap);
    ui->bg_button->setIcon(color_icon);
    ui->FMV_hack->setChecked(Settings::values.FMV_hack);
    ui->AddTicks->setValue(Settings::values.AddTicks);
}

void ConfigureEnhancements::updateShaders(bool anaglyph) {
    ui->shader_combobox->clear();

    if (anaglyph)
        ui->shader_combobox->addItem("dubois (builtin)");
    else
        ui->shader_combobox->addItem("none (builtin)");

    ui->shader_combobox->setCurrentIndex(0);

    for (const auto& shader : OpenGL::GetPostProcessingShaderList(anaglyph)) {
        ui->shader_combobox->addItem(QString::fromStdString(shader));
        if (Settings::values.pp_shader_name == shader)
            ui->shader_combobox->setCurrentIndex(ui->shader_combobox->count() - 1);
    }
}

void ConfigureEnhancements::RetranslateUI() {
    ui->retranslateUi(this);
}

void ConfigureEnhancements::ApplyConfiguration() {
    Settings::values.resolution_factor =
        static_cast<u16>(ui->resolution_factor_combobox->currentIndex());
    Settings::values.FMV_hack = ui->FMV_hack->isChecked();
    Settings::values.AddTicks = ui->AddTicks->value();
    Settings::values.render_3d =
        static_cast<Settings::StereoRenderOption>(ui->render_3d_combobox->currentIndex());
    Settings::values.factor_3d = ui->factor_3d->value();
    Settings::values.pp_shader_name =
        ui->shader_combobox->itemText(ui->shader_combobox->currentIndex()).toStdString();
    Settings::values.filter_mode = ui->toggle_linear_filter->isChecked();
    Settings::values.layout_option =
        static_cast<Settings::LayoutOption>(ui->layout_combobox->currentIndex());
    Settings::values.swap_screen = ui->swap_screen->isChecked();
    Settings::values.dump_textures = ui->toggle_dump_textures->isChecked();
    Settings::values.custom_textures = ui->toggle_custom_textures->isChecked();
    Settings::values.preload_textures = ui->toggle_preload_textures->isChecked();
    Settings::values.bg_red = static_cast<float>(bg_color.redF());
    Settings::values.bg_green = static_cast<float>(bg_color.greenF());
    Settings::values.bg_blue = static_cast<float>(bg_color.blueF());
}

ConfigureEnhancements::~ConfigureEnhancements() {
    delete ui;
}
