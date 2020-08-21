Commands    

    git submodule update --init --recursive

How to compile for Linux-x64

    .\thirdparty\vcpkg\bootstrap-vcpkg.sh

    .\thirdparty\vcpkg\vcpkg.exe install @deps_vcpkg_x64-linux.txt

    ./thirdparty/premake/linux/premake5 gmake2      

    make

How to compile for Window-x64

    .\thirdparty\vcpkg\bootstrap-vcpkg.bat

    .\thirdparty\vcpkg\vcpkg.exe install @deps_vcpkg_x64-windows.txt

    .\thirdparty\vcpkg\vcpkg.exe integrate install

    .\build-vs2019.bat

This project uses 
- SDL2 - window management (cross-platform)
- Glew - opengl extention management (cross-platform)
- ImGui - ui (cross-platform)

planned to use in future
- GameNetworkingSockets - https://github.com/ValveSoftware/GameNetworkingSockets
- GGPO
- Bullet3 - physics
- Entt - entity component system