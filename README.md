[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

[![game_defence_tests](https://github.com/Turtwiggy/game_engine/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/Turtwiggy/game_engine/actions/workflows/windows.yml)

### Install

- c++ compiler (MSVC, g++, clang)
- cmake e.g. `choco install cmake`, `brew install cmake`
- ninja e.g. `choco install ninja`, `brew install ninja`

Windows & Mac & Linux (see triplet in file)

```bash
git clone
git submodule update --init --recursive
thirdparty/vcpkg/bootstrap-vcpkg.bat
thirdparty/vcpkg/vcpkg.exe install @vcpkg-x64-windows.txt
```

Emscripten

```bash
git clone
git submodule update --init --recursive
thirdparty/emsdk/emsdk install latest
thirdparty/emsdk/emsdk activate latest
thirdparty/vcpkg/bootstrap-vcpkg.bat
thirdparty/vcpkg/vcpkg.exe install @vcpkg-wasm32-emscripten.txt
```

### Develop

I develop with visual studio code. You can open the .code-workspace using the C/C++ extention tools:

    C/C++ by Microsoft
    This extension provides IntelliSense support for C/C++.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

    CMake Tools by vector-of-bool
    This extension allows for configuring the CMake project and building it from within the Visual Studio Code IDE.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=vector-of-bool.cmake-tools
