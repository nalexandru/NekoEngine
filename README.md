# NekoEngine

## Description

Cross-platform game engine.

Supported platforms:
* Windows XP & newer
* Linux on x86_64 & arm64
* Mac OS X 10.4 - 10.6. Newer versions should work too, however they aren't tested.

Supported rendering APIs:
* Direct3D 9
* OpenGL 1.5+

Supported audio APIs:
* XAudio2
* OpenAL

Vulkan support in-progress.

This repository is a mirror which is not frequently updated.

## Build instructions

### Windows

#### Prerequisites
* Visual Studio 2019 or Visual C++ Express 2005 (Visual Studio 2005 should work too, however it isn't tested).
* (Optional) FreeType for building the FontGen tool

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build NekoEngine.sln (VS2019) or Compat/vc8/NekoEngine.sln (VC++2005)

### Linux

#### Prerequisites
* gcc
* cmake
* X11, OpenGL & OpenAL development files
* On Debian based systems you need to install the following packages:
	* gcc
	* libx11-dev
	* libopenal-dev
	* libgl-dev
	* cmake

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build the project with CMake

### Mac OS X

#### Prerequisites
* Xcode; The project is in 2.4-compatible format so that it will work on Mac OS X 10.4.

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\build\Data
3. Build NekoEngine.xcodeproj

### Notes
* On Windows the renderer may be selected with the Renderer.sModule option in Engine.ini. The options are: D3D12Render, D3D9Render & GLRender.
* The Mac OS X implementation has been tested on 10.4 PowerPC and 10.6 Intel.

## 3rd Party libraries

* PhysFS - https://icculus.org/physfs/
* Lua - https://www.lua.org/
* cgltf - https://github.com/jkuhlmann/cgltf
* Jsmn - https://github.com/zserge/jsmn
* stb_image, stb_image_write, stb_vorbis - https://github.com/nothings/stb
* uthash - https://troydhanson.github.io/uthash/
* glad - https://glad.dav1d.de/
* volk - https://github.com/zeux/volk
