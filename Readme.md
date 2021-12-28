[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

### Features

- threaded texture loading
- controller support via SDL2
- sweep and prune collision detection for aabb
- batch renderer
- shadow casting
- compile to web via emscripten

### Install

- c++ compiler (MSVC, gcc, clang)
- make

### Build

```
git clone
git submodule update --init --recursive
```

```
(Windows) Building with g++ via mingw64

make -C .
```

```
(Emscripten) Building

thirdparty/emsdk/emsdk activate latest
make -C . -f Makefile-emscripten
```
