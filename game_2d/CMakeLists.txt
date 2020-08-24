#this cmake lists compiles the game_2d with the engine

cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(game_2d VERSION 0.1.0)

message("game_2d: ${CMAKE_SYSTEM_NAME}")
message("game_2d: ${CMAKE_BUILD_TYPE}")

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
    "${CMAKE_SOURCE_DIR}/engine/src/*.cpp"

    #game
    "${CMAKE_SOURCE_DIR}/game_2d/src/*.cpp"

    #imgui
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples/imgui_impl_sdl.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples/imgui_impl_opengl3.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp"
)

add_executable(game_2d ${SRC_FILES})

#Includes
target_include_directories(game_2d
  PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/src
    ${CMAKE_SOURCE_DIR}/game_2d/src
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples
)
#Vcpkg Includes
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_include_directories(game_2d
  PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-windows/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_include_directories(game_2d
    PRIVATE
      ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-linux/include
  )
endif()  

#Link External Libs
target_link_libraries(game_2d PRIVATE EnTT::EnTT)
target_link_libraries(game_2d PRIVATE LinearMath Bullet3Common BulletDynamics BulletSoftBody)
target_link_libraries(game_2d PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(game_2d PRIVATE glm)
target_link_libraries(game_2d PRIVATE GLEW::GLEW)
target_include_directories(game_2d PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(game_2d PRIVATE assimp::assimp)
#Link SDL2
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_link_libraries(game_2d PRIVATE SDL2::SDL2 SDL2::SDL2main)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_link_libraries(game_2d PRIVATE SDL2::SDL2main SDL2::SDL2-static)
endif()  

#create symlink assets for build
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set (source "${CMAKE_SOURCE_DIR}/assets")
  set (destination "${CMAKE_CURRENT_BINARY_DIR}/assets")
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination} 
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
  )
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)