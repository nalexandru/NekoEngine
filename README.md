# NekoEngine

## Build Status

| Windows |
|---------|
|[![Build status](https://ci.appveyor.com/api/projects/status/t8pace9glsyigisd/branch/master?svg=true)](https://ci.appveyor.com/project/nalexandru/nekoengine/branch/master)|

## Description

Cross-platform game engine.

Supported platforms:
* Windows 7 64-bit or newer
* Linux x86_64
* macOS 12.6 or newer arm64
* iOS 14.0 or newer
* FreeBSD 13.0 amd64
* OpenBSD 6.9 amd64

Supported rendering APIs:
* Vulkan 1.2
* Metal 2

This repository is a mirror which is not frequently updated.

## Build instructions

### Windows

#### Prerequisites
* Visual Studio 2022
* Vulkan SDK 1.3.216 or later
* (Optional) FreeType for building the FontGen tool

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build NekoEngine.sln

### *nix

#### Prerequisites
* gcc 4.9+ or clang 3.3+ (any compiler with C11 support should work)
* cmake 3.12+
* X11, Vulkan & OpenAL development files
* shaderc or glslang
* For Arch based systems run the following:
```
pacman -S gcc libx11 vulkan-devel cmake shaderc openal
```
* For FreeBSD:
```
pkg install vulkan-headers vulkan-loader vulkan-tools vulkan-validation-layers cmake shaderc openal-soft
```

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build the project with CMake

### macOS / iOS

#### Prerequisites
* Xcode 14 beta

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\build\Data
3. Build NekoEngine.xcodeproj

#### Notes
* It has only been tested on a M1 Mac Mini / iPhone 11.

## 3rd Party libraries

* PhysFS - https://icculus.org/physfs/
* Lua - https://www.lua.org/
* cgltf - https://github.com/jkuhlmann/cgltf
* Jsmn - https://github.com/zserge/jsmn
* stb_image, stb_image_write, stb_vorbis - https://github.com/nothings/stb
* uthash - https://troydhanson.github.io/uthash/
* volk - https://github.com/zeux/volk
* simple-stdatomic-for-VS-Clang - https://github.com/zenny-chen/simple-stdatomic-for-VS-Clang
* OpenAL Soft - https://github.com/kcat/openal-soft
* meshoptimizer - https://github.com/zeux/meshoptimizer
