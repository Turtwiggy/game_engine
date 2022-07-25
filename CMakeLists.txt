cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_VERBOSE_MAKEFILE on)
if(EMSCRIPTEN)
  message("Compiling for emscripten")

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")
# # add_definitions(-s USE_SDL=2)
# set(CMAKE_EXECUTABLE_SUFFIX ".html")
# set(VCPKG_TARGET_TRIPLET wasm32-emscripten)
# set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/gameengine/thirdparty/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake)
# set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/gameengine/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
# set_target_properties(client PROPERTIES LINK_FLAGS "-s DEMANGLE_SUPPORT=1 --preload-file ${CMAKE_SOURCE_DIR}/assets --bind")
else()
  message("Not compiling for emscripten - adding vcpkg ${CMAKE_SOURCE_DIR}")
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
endif()

project(subprojects VERSION 0.1.0)

# projects
add_subdirectory(src)

# add_subdirectory(tests)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)