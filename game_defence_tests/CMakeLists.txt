cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Note: CMAKE_SYSTEM_NAME should be: Emscripten NOT windows
if(EMSCRIPTEN)
  message("Compiling for emscripten (toolkit set in cmake-kits.json)")

  # https://emscripten.org/docs/tools_reference/emcc.html
  # https://emscripten.org/docs/porting/files/packaging_files.html
  set(EMS "-sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=wav,mp3 -sDISABLE_EXCEPTION_CATCHING=1 ")

  # set(EMS "${EMS} -sUSE_PTHREADS=1")
  # set(LD_FLAGS "${LD_FLAGS} -sPTHREAD_POOL_SIZE=4")
  set(LD_FLAGS "-sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sNO_EXIT_RUNTIME=0 -sASSERTIONS=1")
  set(LD_FLAGS "${LD_FLAGS} -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")
  set(LD_FLAGS "${LD_FLAGS} --preload-file assets --bind")
  set(LD_FLAGS "${LD_FLAGS} --shell-file  assets/emscripten/shell_minimal.html")
  set(LD_FLAGS "${LD_FLAGS} ${EMS}")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++ -O2 ${EMS}")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  message("index VCPKG_CHAINLOAD_TOOLCHAIN_FILE: ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
  message("index CMAKE_EXECUTABLE_SUFFIX: ${CMAKE_EXECUTABLE_SUFFIX}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++")

  if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    add_definitions(-D_DEBUG)
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  message("setting cxx flags for gnu...")

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wpedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat")

  if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    add_definitions(-D_DEBUG)
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt+")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1reportTime") # debug compiler frontend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d2cgsummary") # debug compiler backend???
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:precise")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /W3 /wd4275 /wd4251 /bigobj")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /permissive-")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /sdl /Oi /Ot /Oy /Ob2 /Zi")
endif()

project(game_defence_tests VERSION 0.1.0)
message("game_defence_tests...")

include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
find_packages()
find_package(GTest CONFIG REQUIRED)

include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/optick.cmake)
file(GLOB_RECURSE TEST_SRC_FILES
  ${IMGUI_SOURCE}
  ${OPTICK_SOURCE}
  ${CMAKE_SOURCE_DIR}/src/*.cpp

  # do not inlude main()
  ${CMAKE_SOURCE_DIR}/game_defence/src/helpers/*.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/modules/*.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/actors.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/game_state.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/game.cpp
  ${CMAKE_SOURCE_DIR}/game_defence_tests/src/*.cpp
)

# Test executable
add_executable(game_defence_tests ${TEST_SRC_FILES})
link_libs(game_defence_tests)
target_link_libraries(game_defence_tests PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)
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

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/arm64-osx/include)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include)
endif()

# Code coverage
if(CMAKE_CXX_COMPILER_ID MATCHES GNU AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("setting code coverage options...")
  target_compile_options(game_defence_tests PRIVATE --coverage) # -O0 and -g should already be set
  target_link_options(game_defence_tests PRIVATE --coverage)

  find_program(LCOV_PATH lcov)
  find_program(GENHTML_PATH genhtml)

  if(NOT LCOV_PATH)
    message(FATAL_ERROR "lcov not found!")
  endif()

  if(NOT GENHTML_PATH)
    message(FATAL_ERROR "genhtml not found!")
  endif()

  set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")

  # generate lcov file
  add_custom_target(lcov_generate_coverage
    COMMAND ${LCOV_PATH} --capture --directory .
    COMMAND ${LCOV_PATH} --remove lcov.info '/usr/*' --output-file lcov.info
    COMMAND ${LCOV_PATH} --list lcov.info
    COMMAND ${GENHTML_PATH} lcov.info --output-directory ${COVERAGE_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated at ${COVERAGE_OUTPUT_DIR}/index.html"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating code coverage report..."
    VERBATIM
  )

  # cleanup lcov file
  add_custom_target(lcov_remove_coverage
    COMMAND ${LCOV_PATH} --directory . --zerocounters
    COMMAND ${CMAKE_COMMAND} -E remove lcov.info
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_OUTPUT_DIR}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning up code coverage data..."
    VERBATIM
  )
endif()

message("adding test to ctest..")

# If COMMAND specifies an executable target (created by add_executable)
# it will automatically be replaced by the location of the executable
# created at build time
add_test(NAME all_game_defence_tests COMMAND game_defence_tests)
