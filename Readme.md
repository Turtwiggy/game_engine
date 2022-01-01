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
- cmake e.g. choco install cmake
- ninja e.g. choco install ninja

### Build

Include build_engine.cmake in to the cmake that makes the executable.
e.g. see: CMakeLists.txt at https://github.com/Turtwiggy/opengl_game
