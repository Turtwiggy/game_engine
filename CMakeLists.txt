cmake_minimum_required(VERSION 3.0.0)

set (CMAKE_CXX_STANDARD 17)
#add_definitions("-Wall" "-g")
add_definitions("_CRT_SECURE_NO_WARNINGS")
add_definitions("IMGUI_IMPL_OPENGL_LOADER_GLEW")

#vcpkg e.g.
#-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

#vcpkg VCPKG_TARGET_TRIPLET 
if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

project(fightinggame)

#packages from vcpkg
find_package(SDL2 CONFIG REQUIRED)
find_package(opengl32 CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

include_directories(
    engine/src
    game_3d/src
    thirdparty/imgui
    thirdparty/imgui/examples
    thirdparty/vcpkg/installed/x64-windows/include
)

file(GLOB_RECURSE SRC_FILES
    #engine    
    "engine/src/*"

    #game
    "game_3d/*"

    #imgui
    "thirdparty/imgui/imgui_widgets.cpp"
    "thirdparty/imgui/examples/imgui_impl_sdl.cpp"
    "thirdparty/imgui/examples/imgui_impl_opengl3.cpp"
    "thirdparty/imgui/imgui.cpp"
    "thirdparty/imgui/imgui_draw.cpp"
    "thirdparty/imgui/imgui_demo.cpp"
)

add_executable(fightinggame ${SRC_FILES})

#link packages
target_link_libraries(fightinggame PRIVATE SDL2)
target_link_libraries(fightinggame PRIVATE opengl32)
target_link_libraries(fightinggame PRIVATE glew)
target_link_libraries(fightinggame PRIVATE assimp-vc142-mt)
target_link_libraries(fightinggame PRIVATE fmt)

    