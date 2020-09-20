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
* DirectX Shader Compiler (included with the Windows 10 SDK)
* (Optional) Vulkan driver with support for VK_KHR_ray_tracing
* (Optional) FreeType for building the FontGen tool

### Build instructions

1. Clone the repository
2. Clone the data repository found at https://github.com/nalexandru/NekoEngineData in SOURCE_DIR\bin\Data
3. Build Claire.sln

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
