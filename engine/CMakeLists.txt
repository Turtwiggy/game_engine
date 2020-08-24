#this cmake lists compiles the engine with the engine

cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(engine VERSION 0.1.0)

message("engine: ${CMAKE_SYSTEM_NAME}")
message("engine: ${CMAKE_BUILD_TYPE}")

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

    #imgui
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples/imgui_impl_sdl.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples/imgui_impl_opengl3.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp"
)

add_library(engine ${SRC_FILES})

#Includes
target_include_directories(engine
  PRIVATE
    ${CMAKE_SOURCE_DIR}/engine/src
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples
)
#Vcpkg Includes
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_include_directories(engine
  PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-windows/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_include_directories(engine
    PRIVATE
      ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-linux/include
  )
endif()  

#Link External Libs
target_link_libraries(engine PRIVATE EnTT::EnTT)
target_link_libraries(engine PRIVATE LinearMath Bullet3Common BulletDynamics BulletSoftBody)
target_link_libraries(engine PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(engine PRIVATE glm)
target_link_libraries(engine PRIVATE GLEW::GLEW)
target_include_directories(engine PRIVATE ${STB_INCLUDE_DIRS})
target_link_libraries(engine PRIVATE assimp::assimp)
#Link SDL2
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_link_libraries(engine PRIVATE SDL2::SDL2 SDL2::SDL2main)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_link_libraries(engine PRIVATE SDL2::SDL2main SDL2::SDL2-static)
endif()  

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)