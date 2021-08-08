#this is the master cmake file used by visual studio code to 
#build the individual cmake projects

#"-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"

cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)

project(subprojects VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# projects
#add_subdirectory(examples/hello_application)
#add_subdirectory(examples/net_client)
#add_subdirectory(examples/net_server)
add_subdirectory(examples/game_2d)
# add_subdirectory(examples/game_3d)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)