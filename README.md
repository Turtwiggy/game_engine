[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

[![Windows](https://github.com/Turtwiggy/game_engine/actions/workflows/windows.yml/badge.svg)](https://github.com/Turtwiggy/game_engine/actions/workflows/windows.yml)

[![Mac](https://github.com/Turtwiggy/game_engine/actions/workflows/mac.yml/badge.svg)](https://github.com/Turtwiggy/game_engine/actions/workflows/mac.yml)

[![Linux](https://github.com/Turtwiggy/game_engine/actions/workflows/linux.yml/badge.svg)](https://github.com/Turtwiggy/game_engine/actions/workflows/linux.yml)

### Install prerequisites

- c++ compiler (MSVC, g++, clang)
  - `sudo apt install build-essential`
- cmake e.g. `choco install cmake`, `brew install cmake`, `sudo apt-get install cmake`
- ninja e.g. `choco install ninja`, `brew install ninja`, `sudo apt-get install ninja-build`

Windows

```bash
git clone
git submodule update --init --recursive
thirdparty/vcpkg/bootstrap-vcpkg.bat
thirdparty/vcpkg/vcpkg.exe install @vcpkg-x64-windows.txt
```

Mac

```bash
brew install pkg-config

git clone
git submodule update --init --recursive
thirdparty/vcpkg/bootstrap-vcpkg.sh
thirdparty/vcpkg/vcpkg install @vcpkg-arm64-osx.txt
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

Linux

https://github.com/ValveSoftware/GameNetworkingSockets/blob/master/BUILDING.md#linux

```bash
git clone
git submodule update --init --recursive
sudo apt-get update

# gdb
sudo apt install g++ gdb -y

# vcpkg
sudo apt-get install curl zip unzip tar pkg-config -y
sudo apt install autoconf libtool linux-libc-dev -y

# opengl
sudo apt install libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev -y

# glew
sudo apt-get install libglew-dev libxmu-dev libxi-dev libgl-dev -y

# sdl2
sudo apt install libsdl2-dev libsdl2-2.0-0 -y

# temporary to fix: https://github.com/microsoft/vcpkg/issues/37279
sudo apt install python3-jinja2

thirdparty/vcpkg/bootstrap-vcpkg.sh
thirdparty/vcpkg/vcpkg install @vcpkg-x64-linux.txt

```

### Develop

I develop with visual studio code. You can open the .code-workspace using the C/C++ extention tools:

    C/C++ by Microsoft
    This extension provides IntelliSense support for C/C++.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

    CMake Tools by vector-of-bool
    This extension allows for configuring the CMake project and building it from within the Visual Studio Code IDE.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=vector-of-bool.cmake-tools

For a more complete list see [.vscode/extensions.json](.vscode/extensions.json)
