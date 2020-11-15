#this is the master cmake file used by visual studio code to 
#build the individual cmake projects

#"-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"

cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(subprojects VERSION 0.1.0)

#add_subdirectory(engine)
add_subdirectory(game_2d)
add_subdirectory(game_3d)

#random c++ projects
#add_subdirectory(experimental_networking)

#examples
add_subdirectory(examples/hello_application)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)