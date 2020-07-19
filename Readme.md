
How to compile

1) Run the .bat file in include/vcpkg/bootstrap-vcpkg.bat
2) install deps in root of project "../../vcpkg.exe @deps_vcpkg_x64-windows"
2) [Build ANGLE](https://github.com/google/angle/blob/master/doc/DevSetup.md)
3) run premake (build.bat) to generate vs 2019 files

This project uses 
- sdl2 - window management (cross-platform)
- imgui - ui (cross-platform)
- Valve's gamenetworkingsockets - networking (unfortunately windows only)
- entt - entity component system
- bullet3 - physics
- ANGLE

Manual deps required to build:
- GameNetworkingSockets - https://github.com/ValveSoftware/GameNetworkingSockets
- ANGLE - https://github.com/google/angle/blob/master/doc/DevSetup.md
