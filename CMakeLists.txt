#this cmake lists compiles the game_3d with the engine

cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(main VERSION 0.1.0)

find_package(SDL2 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(assimp CONFIG REQUIRED)

file(GLOB_RECURSE SRC_FILES
    #engine    
    "engine/src/*.cpp"

    #game
    "game_3d/src/*.cpp"

    #imgui
    "thirdparty/imgui/imgui_widgets.cpp"
    "thirdparty/imgui/examples/imgui_impl_sdl.cpp"
    "thirdparty/imgui/examples/imgui_impl_opengl3.cpp"
    "thirdparty/imgui/imgui.cpp"
    "thirdparty/imgui/imgui_draw.cpp"
    "thirdparty/imgui/imgui_demo.cpp"
)

add_executable(main ${SRC_FILES})

target_include_directories(main
  PRIVATE
    ${PROJECT_SOURCE_DIR}/engine/src
    ${PROJECT_SOURCE_DIR}/game_3d/src
    ${PROJECT_SOURCE_DIR}/thirdparty/imgui
    ${PROJECT_SOURCE_DIR}/thirdparty/imgui/examples
    ${PROJECT_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-windows/include
)

#Link External Libs
target_link_libraries(main PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(main PRIVATE EnTT::EnTT)
target_link_libraries(main PRIVATE LinearMath Bullet3Common BulletDynamics BulletSoftBody)
target_link_libraries(main PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(main PRIVATE glm)
target_link_libraries(main PRIVATE GLEW::GLEW)
target_include_directories(main PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(main PRIVATE assimp::assimp)

#create symlink assets for build
set (source "${CMAKE_SOURCE_DIR}/assets")
set (destination "${CMAKE_CURRENT_BINARY_DIR}/Debug/assets")
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination} 
  DEPENDS ${destination}
  COMMENT "symbolic link resources folder from ${source} => ${destination}"
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)