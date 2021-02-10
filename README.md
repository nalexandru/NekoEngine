# NekoEngine

## Description

Cross-platform game engine.

Supported platforms:
* Windows Vista or newer
* Linux on x86_64 & arm64
* macOS 11.0 x86_64 & arm64

Supported rendering APIs:
* Vulkan (in-progress)
* Metal (in-progress)

Supported audio APIs:
* XAudio2
* OpenAL

This repository is a mirror which is not frequently updated.

## Build instructions

### Windows

#### Prerequisites
* Visual Studio 2019
* (Optional) Vulkan driver with support for VK_KHR_ray_tracing
* (Optional) FreeType for building the FontGen tool

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build NekoEngine.sln

### Linux

#### Prerequisites
* gcc
* cmake
* X11, Vulkan & OpenAL development files
* On Debian based systems you need to install the following packages:
	* gcc
	* libx11-dev
	* libopenal-dev
	* libvulkan-dev
	* vulkan-headers
	* vulkan-validationlayers
	* cmake

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build the project with CMake

### Mac OS X

#### Prerequisites
* Xcode

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\build\Data
3. Build NekoEngine.xcodeproj

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
