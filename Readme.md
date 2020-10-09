Download external libs

    git submodule update --init --recursive

(Linux) Install vcpkg dependancies for Linux-x64

    ./thirdparty/vcpkg/bootstrap-vcpkg.sh

    ./thirdparty/vcpkg/vcpkg.exe install @deps_vcpkg_x64-linux.txt

(Windows) Install vcpkg dependancies for Window-x64

    .\thirdparty\vcpkg\bootstrap-vcpkg.bat

    .\thirdparty\vcpkg\vcpkg.exe install @deps_vcpkg_x64-windows.txt

How to build a project

    - Open the .workspace file in visual studio code

    - Install C/C++ extention

    - Install CMake Tools

    - Build the desired project

This project uses 
- SDL2 - window management (cross-platform)
- Glew - opengl extention management (cross-platform)
- ImGui - ui (cross-platform)
- Entt - entity component system

planned to use in future
- GameNetworkingSockets - https:\\github.com\ValveSoftware\GameNetworkingSockets
- GGPO
- Bullet3 - physics
