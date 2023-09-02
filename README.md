# ZygiskUnityCriwareKeylogger

> [Zygisk](https://github.com/topjohnwu/Magisk) part of Magisk allows you to run code in every Android application's Process.


## Introduction

[ZygiskUnityCriwareKeylogger](README.md) is a zygisk module for extracting criware encryption keys
in unity games.

This repo also provides a [Riru](https://github.com/RikkaApps/Riru) flavor in case you are still
using riru with an older magisk version rather than zygisk.

## How to use the module

### Quick start
- Download the latest release from the [Release Page](https://github.com/lico-n/ZygiskUnityCriwareKeylogger/releases)\
  If you are using riru instead of zygisk choose the riru-release. Otherwise choose the normal version.
- Transfer the zip file to your device and install it via Magisk.
- Reboot after install
- Launch your unity game
- The extracted key is logged to `adb logcat -s ZygiskUnityCriwareKeylogger`

## How to build

- Checkout the project
- Run `./gradlew :module:assembleRelease`
- The build magisk module should then be in the `out` directory.

You can also build and install the module to your device directly with `./gradlew :module:flashAndRebootZygiskRelease`

## Notes

#### Can I keep it enabled all the times?

You can but it does have a slight performance impact and can cause issues with unity games with modified engine.
So I would recommend to only enable the module whenever you want to extract a key and otherwise keep it disabled.

#### Does it work on emulators?

Yes, but it is only tested on 64-bit ldplayer, nox and memu. Other emulators might or might not work.
In case your emulator crashes at startup, please create an issue with logs `adb logcat -s ZygiskUnityCriwareKeylogger`
and it should possible to add support.


## Credits

- The il2cpp api was mostly taken from https://github.com/Perfare/Zygisk-Il2CppDumper

