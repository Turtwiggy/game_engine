[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

### Install

- c++ compiler (MSVC, gcc, clang)
- cmake e.g. `choco install cmake`
- ninja e.g. `choco install ninja`

```bash
git clone
git submodule update --init --recursive
thirdparty\vcpkg\bootstrap-vcpkg.bat
thirdparty\vcpkg\vcpkg.exe --feature-flags=versions install --triplet x64-windows
```

### Develop

I develop with visual studio code. You can open the .code-workspace using the C/C++ extention tools:

    C/C++ by Microsoft
    This extension provides IntelliSense support for C/C++.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

    CMake Tools by vector-of-bool
    This extension allows for configuring the CMake project and building it from within the Visual Studio Code IDE.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=vector-of-bool.cmake-tools
