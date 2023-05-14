# NekoEngine

## Description

Cross-platform game engine.

Supported platforms:
* Windows 10 or newer*
* Linux x86_64
* macOS 13.0 or newer arm64
* iOS 16.0 or newer
* FreeBSD 13.2 amd64
* OpenBSD 7.3 amd64

\* Windows 7/8/8.1 will work with NVIDIA GPUs. AMD GPUs will not because the driver doesn't support bufferDeviceAddress.

Supported rendering APIs:
* Vulkan 1.2
* Metal 2

This repository is a mirror which is not frequently updated.

## Build instructions

CMake can also be used for Windows.

### Windows

#### Prerequisites
* Visual Studio 2022 17.5 or later
* Vulkan SDK 1.3.216 or later (for shaderc)
* Python 3

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Open the x64 Native Tools Command Prompt, cd to the source directory and run build_deps.py
3. Build NekoEngine.sln

### *nix

#### Prerequisites
* gcc 11+ or clang 13+ (any compiler with C17 & C++23 support should work)
* cmake 3.13+
* X11, XInput2, Vulkan, OpenAL development files
* shaderc or glslang
* python 3

* For Arch based systems run the following:
```
pacman -S gcc libbsd libx11 libxi cmake shaderc openal python3
```
* For FreeBSD:
```
pkg install cmake shaderc openal-soft xinput python3 curl
```
* For OpenBSD:
```
pkg_add cmake glslang openal python3
```

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Run SOURCE_DIR\build_deps.py
4. Build the project with CMake

### macOS / iOS

#### Prerequisites
* Xcode 14.3

#### Build
1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Run SOURCE_DIR\build_deps.py*
4. Build NekoEngine.xcodeproj

\* To build the dependencies for iOS run the script as
```
python build_deps.py iOS arm64
```

#### Notes
* It has only been tested on M1 machines / iPhone 11.

## 3rd Party libraries included in the source

* PhysFS* - https://icculus.org/physfs/
* Lua* - https://www.lua.org/
* cgltf - https://github.com/jkuhlmann/cgltf
* jsmn - https://github.com/zserge/jsmn
* stb_image, stb_image_write - https://github.com/nothings/stb
* uthash - https://troydhanson.github.io/uthash/
* volk* - https://github.com/zeux/volk
* DirectXMath - https://github.com/microsoft/DirectXMath
* DirectX Headers - https://github.com/microsoft/DirectX-Headers
* Vulkan Headers - https://github.com/KhronosGroup/Vulkan-Headers
* tinyobjloader - https://github.com/tinyobjloader/tinyobjloader

\* Some modifications were made to the library.
