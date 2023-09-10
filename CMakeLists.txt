cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_VERBOSE_MAKEFILE on)
if(EMSCRIPTEN)
else()
  message("Not compiling for emscripten - adding vcpkg ${CMAKE_SOURCE_DIR}")
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

project(subprojects VERSION 0.1.0)

# projects
add_subdirectory(game_3d)
add_subdirectory(game_defence)
add_subdirectory(game_defence_tests)
add_subdirectory(game_networked)

# add_subdirectory(tests)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)