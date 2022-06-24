[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

### Engine Components

- SDL2/opengl
- imgui/opengl
- compile to web via emscripten

### Install

- c++ compiler (MSVC, gcc, clang)
- cmake e.g. `choco install cmake`
- ninja e.g. `choco install ninja`

### Install VCPKG deps

`.\thirdparty\vcpkg\bootstrap-vcpkg.bat`
`.\thirdparty\vcpkg\vcpkg.exe install @vcpkg_x64-windows.txt`

### Build

Include build_engine.cmake in to the cmake that makes the executable.
e.g. https://github.com/Turtwiggy/opengl_game/blob/main/game/CMakeLists.txt
