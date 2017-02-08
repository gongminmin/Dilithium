# Dilithium

Dilithim is an open source library for converting DXIL and SPIR-V in bidirectional. Currently it's in a very early stage.

## Concepts

### What is DXIL
DXIL (DirectX Intermediate Language) is a binary format that introduced by [DirectX Shader Compiler](https://github.com/Microsoft/DirectXShaderCompiler). As the name suggests, it's an intermediate language compiled from HLSL, and consumed by GPU drivers. Unlike its predecessor DXBC (DirectX Byte Code), DXIL is open and standardized.

For more information, see the [DXIL.rst](https://github.com/Microsoft/DirectXShaderCompiler/blob/master/docs/DXIL.rst).

### What is SPIR-V
SPIR-V is also a binary intermediate language, for GPU shaders in OpenGL and OpenGLES ecosystem. It's part of OpenCL 2.1 core as well as Vulkan core.

For more information, see the [SPIR-V spec](https://www.khronos.org/registry/spir-v/specs/1.0/SPIRV.html).

### Why need a converter
In D3D, HLSL shaders are compiled to DXIL/DXBC. Meanwhile in OpenGL/OpenGLES, GLSL/ESSL shaders are compiled to SPIR-V. But those two intermediate languages can't talk to each other. Developing an cross-API app means you need to write at least 2 copies of every shaders, one in HLSL, one in GLSL/ESSL. An automatic converter can simplify this process, only write in one high-level shader language and use them in all places. Converting in binary ILs, instead of high-level languages, reduces parsing time after time, and keeps the optimization.

## Building

Before you build, you will need to have some additional software installed.

* [Git](http://git-scm.com/downloads).
* [CMake](https://cmake.org/). Version 3.4 or up is required. You need not change your PATH variable during installation.
* [Python](https://www.python.org/downloads/). Version 2.7 or up is required. You need not change your PATH variable during installation.

### Instructions
[TBD]

### Directory Structure
* External/: Intended location for external dependencies.
* Include/: Library clients should add this directory to the include search path.
* Src/: Library implementation.
* Tools/: Command line executables.

## Contributing
As an open source project, Dilithium benefits greatly from both the volunteer work of helpful developers and good bug reports made by users. 

### Bug Reports & Feature Requests
If you've noticed a bug or have an idea that you'd like to see come real, why not work on it? Bug reports and feature requests are typically submitted to the issue tracker https://github.com/gongminmin/Dilithium/issues.

## Why this name
In Star Trek, Dilithium is a material which serves as a controlling agent in the faster-than-light warp drive. It is used to contain and regulate the annihilation reaction of matter and antimatter in a starship's warp core, which otherwise would explode from the uncontrolled annihilation reaction. In this project, Dilithium contains and regulates HLSL/DXIL and GLSL/SPIR-V in a game engine.

## Links
[Dilithium project page](https://github.com/gongminmin/Dilithium)
[DirectX Shader Compiler](https://github.com/Microsoft/DirectXShaderCompiler)
[SPIRV-Cross, a tool for parsing SPIR-V](https://github.com/KhronosGroup/SPIRV-Cross)
