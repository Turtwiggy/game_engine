Download external libs

    git submodule update --init --recursive

(Linux) Install vcpkg dependancies for Linux-x64

    ./thirdparty/vcpkg/bootstrap-vcpkg.sh
    ./thirdparty/vcpkg/vcpkg.exe install @deps_vcpkg_x64-linux.txt

(Windows) Install vcpkg dependancies for Window-x64

    .\thirdparty\vcpkg\bootstrap-vcpkg.bat
    .\thirdparty\vcpkg\vcpkg.exe install @deps_vcpkg_x64-windows.txt    

This project uses 

    - SDL2 - window management (cross-platform)
    - Glew - opengl extention management (cross-platform)
    - ImGui 
    - Entt - entity component system

planned to use in future

    - GGPO
    - Bullet3 - physics

How to build with Visual Studio Code

    First, install the tools listed below for VS Code

    C/C++ by Microsoft
    This extension provides IntelliSense support for C/C++.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

    CMake Tools by vector-of-bool
    This extension allows for configuring the CMake project and building it from within the Visual Studio Code IDE.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=vector-of-bool.cmake-tools

    - Open the .workspace file in visual studio code