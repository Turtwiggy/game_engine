1) Run the .bat file in include/vcpkg/bootstrap-vcpkg.bat
2) install deps in root of project "../../vcpkg.exe @deps_vcpkg_x64-windows"
2) build gamenetworkingsockets for windows (good luck)
3) run premake (build.bat) to generate vs 2019 files

This project is using 
sdl2 - window management (cross-platform)
bgfx - rendering library (cross-platform)
imgui - ui (cross-platform)
Valve's gamenetworkingsockets - networking (unfortunately windows only)
entt - entity component system
bullet3 - physics

undecided libs
audio (sfml, openal?)