# NekoEngine

## Build Status

| Windows | macOS |
|---------|-------|
|[![Build status](https://ci.appveyor.com/api/projects/status/t8pace9glsyigisd/branch/master?svg=true)](https://ci.appveyor.com/project/nalexandru/nekoengine/branch/master)|![Build status](https://github.com/nalexandru/NekoEngine/actions/workflows/xcode.yml/badge.svg?branch=master&event=push)|

## Description

Cross-platform game engine.

Supported platforms:
* Windows 7 64-bit or newer*
* Linux x86_64
* macOS 11.0+ arm64 (x86_64 with a Metal 3 GPU should work too)
* iOS 14.4+
* FreeBSD 13.0 amd64
* OpenBSD 6.9 amd64

  \* Windows 7 is supported only on AMD GPUs, for Nvidia Windows 10 is required (there is no heap that is host coherent and device local exposed on Windows 7, 8 or 8.1 by Nvidia drivers).
  
Supported rendering APIs:
* Vulkan 1.2
* Metal 3

This repository is a mirror which is not frequently updated.

## Build instructions

### Windows

#### Prerequisites
* Visual Studio 2019
* Vulkan SDK
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
pacman -S gcc libx11 vulkan-devel openal cmake shaderc
```

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build the project with CMake

### macOS / iOS

#### Prerequisites
* Xcode 12

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
* glad - https://glad.dav1d.de/
* volk - https://github.com/zeux/volk
* simple-stdatomic-for-VS-Clang - https://github.com/zenny-chen/simple-stdatomic-for-VS-Clang
* D3D12 Helper Library - https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Libraries/D3DX12
