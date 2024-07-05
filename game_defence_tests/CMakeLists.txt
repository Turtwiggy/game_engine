# this cmake lists compiles the game_defence_tests with the engine

cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)

if(EMSCRIPTEN)
  message("Compiling for emscripten (toolkit set in cmake-kits.json)")

  # https://emscripten.org/docs/tools_reference/emcc.html
  # https://emscripten.org/docs/porting/files/packaging_files.html
  set(EMS "-sUSE_SDL=2 -sDISABLE_EXCEPTION_CATCHING=1")
  set(LD_FLAGS "-sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sNO_EXIT_RUNTIME=0 -sASSERTIONS=1")
  set(LD_FLAGS "${LD_FLAGS} -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")
  set(LD_FLAGS "${LD_FLAGS} -sDEMANGLE_SUPPORT=1 --preload-file assets --bind")
  set(LD_FLAGS "${LD_FLAGS} --shell-file assets/emscripten/shell_minimal.html")
  set(LD_FLAGS "${LD_FLAGS} ${EMS}")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++ ${EMS}")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  message("index VCPKG_CHAINLOAD_TOOLCHAIN_FILE: ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
  message("index CMAKE_EXECUTABLE_SUFFIX: ${CMAKE_EXECUTABLE_SUFFIX}")
endif()

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  message("setting cxx flags for mac...")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++")
endif()

if(CMAKE_SYSTEM_NAME MATCHES Linux)
  message("setting cxx flags for linux...")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  add_definitions("-Wall")
  add_definitions("-Wextra")
  add_definitions("-Wformat")
  add_definitions("-O2")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt+")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1reportTime") # debug compiler frontend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d2cgsummary") # debug compiler backend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}/WX /W3 /wd4275 /wd4251")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:fast")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /permissive-")
endif()

project(game_defence_tests VERSION 0.1.0)
message("game_defence_tests: CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message("game_defence_tests: CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message("game_defence_tests: C: ${CMAKE_C_COMPILER_ID}")
message("game_defence_tests: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_defence_tests: CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("game_defence_tests: CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("game_defence_tests: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_defence_tests: source_dir: ${CMAKE_SOURCE_DIR}")
message("index CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

get_packages()

# Add Testing
enable_testing()
find_package(GTest CONFIG REQUIRED)

# Add source files
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/optick.cmake)
file(GLOB_RECURSE SRC_FILES
  ${IMGUI_SOURCE}
  ${OPTICK_SOURCE}
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/helpers/*.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/modules/*.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/actors.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/game_state.cpp
  ${CMAKE_SOURCE_DIR}/game_defence_tests/src/*.cpp
)

add_executable(game_defence_tests
  ${SRC_FILES}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/arm64-osx/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include
  )
endif()

# includes
target_include_directories(game_defence_tests PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${OPTICK_INCLUDES}
  ${Stb_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/game_defence/src
  ${CMAKE_SOURCE_DIR}/game_defence_tests/src
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/single_include
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui-filebrowser
)

link_libs(game_defence_tests)

target_link_libraries(game_defence_tests PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)