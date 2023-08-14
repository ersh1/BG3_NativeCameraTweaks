# BG3_NativeCameraTweaks
 Native dll plugin providing several camera tweaks for Baldur's Gate 3.

 [Nexus Mods page](https://www.nexusmods.com/baldursgate3/mods/945)

## Requirements

- [CMake](https://cmake.org/)
  - Add this to your `PATH`
- [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
- [Vcpkg](https://github.com/microsoft/vcpkg)
  - Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
- [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
  - Desktop development with C++
- [Baldur's Gate 3 Steam Distribution](https://store.steampowered.com/app/1086940/Baldurs_Gate_3/)
  - Add the environment variable `BG3PATH` with the value as path to game install folder
  
## Register Visual Studio as a Generator

- Open `x64 Native Tools Command Prompt`
- Run `cmake`
- Close the cmd window

## Building

```
git clone https://github.com/gottyduke/BG3_PluginTemplate.git
cd BG3_NativeCameraTweaks
git submodule init
git submodule update
.\build-release.ps1
```

## License

[GPL-3.0-or-later](COPYING) WITH [Modding Exception AND GPL-3.0 Linking Exception (with Corresponding Source)](EXCEPTIONS). Specifically, the Modded Code is Baldur's Gate 3 (and its variants) and Modding Libraries include [Native Mod Loader](https://github.com/kassent/DLLPluginLoader) and [DKUtil](https://github.com/gottyduke/DKUtil/) (and variants).

## Credits

- [Kassent's NativeModLoader](https://www.nexusmods.com/baldursgate3/mods/944/) ([GitHub](https://github.com/kassent/DLLPluginLoader))
- [gottyduke](https://github.com/gottyduke) for [DKUtil](https://github.com/gottyduke/DKUtil/)
- [Ryan for his CommonLibSSE code](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE) which was referenced in DKUtil.
