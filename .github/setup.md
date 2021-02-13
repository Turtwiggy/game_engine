### How to get setup

    git clone https://github.com/Turtwiggy/Cplusplus_game_engine.git
    git submodule update --init --recursive

(Linux) Install vcpkg dependancies for Linux-x64

    ./thirdparty/vcpkg/bootstrap-vcpkg.sh
    ./thirdparty/vcpkg/vcpkg install @deps_vcpkg_x64-linux.txt
    ./thirdparty/vcpkg/vcpkg --overlay-ports=./thirdparty/GameNetworkingSockets/vcpkg_ports/ install gamenetworkingsockets:x64-linux

(Windows) Install vcpkg dependancies for Window-x64

    .\thirdparty\vcpkg\bootstrap-vcpkg.bat
    .\thirdparty\vcpkg\vcpkg.exe install @deps_vcpkg_x64-windows.txt    
    .\thirdparty\vcpkg\vcpkg.exe --overlay-ports=.\thirdparty\GameNetworkingSockets\vcpkg_ports\ install gamenetworkingsockets:x64-windows

### Building
    
I develop with visual studio code. You can open the .code-workspace using the C/C++ extention tools:

    C/C++ by Microsoft
    This extension provides IntelliSense support for C/C++.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

    CMake Tools by vector-of-bool
    This extension allows for configuring the CMake project and building it from within the Visual Studio Code IDE.
    VS Marketplace Link: https://marketplace.visualstudio.com/items?itemName=vector-of-bool.cmake-tools

Software needed
    
- compiler (MSVC, gcc, etc)
- ninja
- cmake