[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

### Features

- threaded texture loading
- controller support via SDL2
- sweep and prune collision detection for aabb
- batch renderer
- shadow casting

### How to get setup

Software needed

- c++ compiler (MSVC, gcc, clang)
- make

```
git clone
git submodule update --init --recursive

(Linux) Install vcpkg dependancies for Linux-x64

    ./thirdparty/vcpkg/bootstrap-vcpkg.sh
    ./thirdparty/vcpkg/vcpkg install @vcpkg_x64-linux.txt
    ./thirdparty/vcpkg/vcpkg --overlay-ports=./thirdparty/GameNetworkingSockets/vcpkg_ports/ install gamenetworkingsockets:x64-linux

(Windows) Install vcpkg dependancies for Window-x64

    .\thirdparty\vcpkg\bootstrap-vcpkg.bat
    .\thirdparty\vcpkg\vcpkg.exe install @vcpkg_x64-windows.txt
    .\thirdparty\vcpkg\vcpkg.exe --overlay-ports=.\thirdparty\GameNetworkingSockets\vcpkg_ports\ install gamenetworkingsockets:x64-windows

(Emscripten) Install vcpkg dependancies for wasm32-emscripten

    .\thirdparty\emsdk\emsdk activate latest
    .\thirdparty\vcpkg\bootstrap-vcpkg.bat
    .\thirdparty\vcpkg\vcpkg.exe install @vcpkg_x32-wasm.txt

```

```
(Emscripten) Building

    thirdparty/emsdk/emsdk activate latest
    make -C . -f Makefile-emscripten

(Windows) Building with gcc

    make -C .

```
