# NekoEngine

## Description

Cross-platform game engine with support for Windows 7+ & Xbox One (UWP).

Supported rendering APIs:
* Direct3D 12 with DXR
* Direct3D 9
* OpenGL 4.5+

Supported audio APIs:
* XAudio2
* OpenAL

Linux, Mac OS X & Vulkan support in-progress.

This repository is a mirror which is not frequently updated.

## Howto

### Prerequisites
* Visual Studio 2019
* DirectX Shader Compiler from https://github.com/Microsoft/DirectXShaderCompiler. The version included with the Windows 10 SDK (version 19041 as of this writing) doesn't define __DXC_VERSION_RELEASE and will result in a failed build while compiling the shaders.
* (Optional) Vulkan driver with support for VK_KHR_ray_tracing
* (Optional) FreeType for building the FontGen tool

### Build instructions

1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build NekoEngine.sln

### Notes
* The D3D12 renderer only supports DXR at the moment, so a GPU with HW ray tracing is required.
* UWP support is experimental and will most likely not work. The Data directory must be copied to SOURCE_DIR\bin\<Configuration>\AppX because of the limitations imposed by the platform.

## 3rd Party libraries

* PhysFS - https://icculus.org/physfs/
* moonjit - https://github.com/moonjit/moonjit
* cgltf - https://github.com/jkuhlmann/cgltf
* Jsmn - https://github.com/zserge/jsmn
* stb_image, stb_image_write, stb_vorbis - https://github.com/nothings/stb
* uthash - https://troydhanson.github.io/uthash/
* glad - https://glad.dav1d.de/
* volk - https://github.com/zeux/volk
* D3D12 Helper Library - https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Libraries/D3DX12
