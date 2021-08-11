# Merge log

Scroll down for the original README.md!

Base revision: c40871f126a5942b16b8b240d256b59e25b5e7ae

|Pull Request|Commit|Title|Author|Merged?|
|----|----|----|----|----|
|[5829](https://github.com/citra-emu/citra/pull/5829)|[6be0cbd05](https://github.com/citra-emu/citra/pull/5829/files/)|fix(bps): Fixes BPS patch changing target size|[fadillzzz](https://github.com/fadillzzz)|Yes|
|[5823](https://github.com/citra-emu/citra/pull/5823)|[bc7f0e7a6](https://github.com/citra-emu/citra/pull/5823/files/)|Android: Backport easy stuff|[SachinVin](https://github.com/SachinVin)|Yes|
|[5804](https://github.com/citra-emu/citra/pull/5804)|[adac0d193](https://github.com/citra-emu/citra/pull/5804/files/)|Minor fx|[weihuoya](https://github.com/weihuoya)|Yes|
|[5786](https://github.com/citra-emu/citra/pull/5786)|[940ec70f1](https://github.com/citra-emu/citra/pull/5786/files/)|Request Camera Permission on MacOS|[vitor-k](https://github.com/vitor-k)|Yes|
|[5785](https://github.com/citra-emu/citra/pull/5785)|[22e4c71bb](https://github.com/citra-emu/citra/pull/5785/files/)|Disable disk shader cache when GL_ARB_get_program_binary is unsupported|[SachinVin](https://github.com/SachinVin)|Yes|
|[5448](https://github.com/citra-emu/citra/pull/5448)|[996ca25a2](https://github.com/citra-emu/citra/pull/5448/files/)|Implement basic rerecording features|[zhaowenlan1779](https://github.com/zhaowenlan1779)|Yes|
|[5411](https://github.com/citra-emu/citra/pull/5411)|[3604df48e](https://github.com/citra-emu/citra/pull/5411/files/)|dumping/ffmpeg_backend: Various fixes|[zhaowenlan1779](https://github.com/zhaowenlan1779)|Yes|
|[5278](https://github.com/citra-emu/citra/pull/5278)|[2c0cf5106](https://github.com/citra-emu/citra/pull/5278/files/)|Port yuzu-emu/yuzu#3791: "configuration: Add Restore Default and Clear options to hotkeys"|[FearlessTobi](https://github.com/FearlessTobi)|Yes|


End of merge log. You can find the original README.md below the break.

------

**BEFORE FILING AN ISSUE, READ THE RELEVANT SECTION IN THE [CONTRIBUTING](https://github.com/citra-emu/citra/wiki/Contributing#reporting-issues) FILE!!!**

Citra
==============
[![GitHub Actions Build Status](https://github.com/citra-emu/citra/workflows/citra-ci/badge.svg)](https://github.com/citra-emu/citra/actions)
[![Bitrise CI Build Status](https://app.bitrise.io/app/4ccd8e5720f0d13b/status.svg?token=H32TmbCwxb3OQ-M66KbAyw&branch=master)](https://app.bitrise.io/app/4ccd8e5720f0d13b)
[![Discord](https://img.shields.io/discord/220740965957107713?color=%237289DA&label=Citra&logo=discord&logoColor=white)](https://discord.gg/FAXfZV9)

Citra is an experimental open-source Nintendo 3DS emulator/debugger written in C++. It is written with portability in mind, with builds actively maintained for Windows, Linux and macOS.

Citra emulates a subset of 3DS hardware and therefore is useful for running/debugging homebrew applications, and it is also able to run many commercial games! Some of these do not run at a playable state, but we are working every day to advance the project forward. (Playable here means compatibility of at least "Okay" on our [game compatibility list](https://citra-emu.org/game).)

Citra is licensed under the GPLv2 (or any later version). Refer to the license.txt file included. Please read the [FAQ](https://citra-emu.org/wiki/faq/) before getting started with the project.

Check out our [website](https://citra-emu.org/)!

Need help? Check out our [asking for help](https://citra-emu.org/help/reference/asking/) guide.

For development discussion, please join us on our [Discord server](https://citra-emu.org/discord/) or at #citra-dev on libera.

### Releases

Citra has two main release channels: Nightly and Canary.

The [Nightly](https://github.com/citra-emu/citra-nightly) build is based on the master branch, and contains already reviewed and tested features.

The [Canary](https://github.com/citra-emu/citra-canary) build is based on the master branch, but with additional features still under review. PRs tagged `canary-merge` are merged only into the Canary builds.

Both builds can be installed with the installer provided on the [website](https://citra-emu.org/download/), but those looking for specific versions or standalone releases can find them in the release tabs of the [Nightly](https://github.com/citra-emu/citra-nightly/releases) and [Canary](https://github.com/citra-emu/citra-canary/releases) repositories.

Currently, development and releases of the Android version are in [a separate repository](https://github.com/citra-emu/citra-android).

A Flatpak for Citra is available on [Flathub](https://flathub.org/apps/details/org.citra_emu.citra). Details on the build process can be found in [our Flathub repository](https://github.com/flathub/org.citra_emu.citra).

### Development

Most of the development happens on GitHub. It's also where [our central repository](https://github.com/citra-emu/citra) is hosted.

If you want to contribute please take a look at the [Contributor's Guide](https://github.com/citra-emu/citra/wiki/Contributing) and [Developer Information](https://github.com/citra-emu/citra/wiki/Developer-Information). You should also contact any of the developers in the forum in order to know about the current state of the emulator because the [TODO list](https://docs.google.com/document/d/1SWIop0uBI9IW8VGg97TAtoT_CHNoP42FzYmvG1F4QDA) isn't maintained anymore.

If you want to contribute to the user interface translation, please check out the [citra project on transifex](https://www.transifex.com/citra/citra). We centralize the translation work there, and periodically upstream translations.

### Building

* __Windows__: [Windows Build](https://github.com/citra-emu/citra/wiki/Building-For-Windows)
* __Linux__: [Linux Build](https://github.com/citra-emu/citra/wiki/Building-For-Linux)
* __macOS__: [macOS Build](https://github.com/citra-emu/citra/wiki/Building-for-macOS)


### Support
We happily accept monetary donations or donated games and hardware. Please see our [donations page](https://citra-emu.org/donate/) for more information on how you can contribute to Citra. Any donations received will go towards things like:
* 3DS consoles for developers to explore the hardware
* 3DS games for testing
* Any equipment required for homebrew
* Infrastructure setup

We also more than gladly accept used 3DS consoles! If you would like to give yours away, don't hesitate to join our [Discord server](https://citra-emu.org/discord/) and talk to bunnei.
