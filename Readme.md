
How to compile for Window-x64

1) Build Vcpkg 

    Windows

    ```./thirdparty/vcpkg/bootstrap-vcpkg.bat```


2) install deps

    ```./thirdparty/vcpkg.exe install @deps_vcpkg_x64-windows```

3) build .sln files with premake

    ``` ./build-vs2019.bat ```

This project uses 
- SDL2 - window management (cross-platform)
- Glew - opengl extention management (cross-platform)
- ImGui - ui (cross-platform)
- Entt - entity component system
- Bullet3 - physics

Libs included but not used
- GameNetworkingSockets - https://github.com/ValveSoftware/GameNetworkingSockets
- GGPO