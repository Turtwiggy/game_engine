
How to compile

1) Run the .bat file in include/vcpkg/bootstrap-vcpkg.bat
2) install deps in root of project "../../vcpkg.exe @deps_vcpkg_x64-windows"
3) run premake (build.bat) to generate vs 2019 files

This project uses 
- sdl2 - window management (cross-platform)
- imgui - ui (cross-platform)
- entt - entity component system
- bullet3 - physics

Manual deps required to build:
- GameNetworkingSockets - https://github.com/ValveSoftware/GameNetworkingSockets
