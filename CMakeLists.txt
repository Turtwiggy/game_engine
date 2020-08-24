#this cmake lists compiles the game_3d with the engine

cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(subprojects VERSION 0.1.0)

add_subdirectory(game_3d)
add_subdirectory(game_2d)
add_subdirectory(engine)
add_subdirectory(ray_tracing_one_weekend)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)