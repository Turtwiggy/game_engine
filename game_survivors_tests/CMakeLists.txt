cmake_minimum_required(VERSION 3.30.0)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-clang.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-emscripten.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-gcc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-msvc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/steam.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/box2d.cmake)

project(game_survivors_tests VERSION 0.1.0)
message("game_survivors_tests...")

# stop the tests trying to launch as a steam game
add_compile_definitions(USE_STEAM=0)
add_compile_definitions(TESTS=1)

find_packages()
find_package(GTest CONFIG REQUIRED)

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.11.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)

file(GLOB_RECURSE TEST_SRC_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/**/*.cpp"
  ${IMGUI_SOURCE}
  ${CMAKE_SOURCE_DIR}/thirdparty/glad/include/glad/glad.c

  # do not inlude main()
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/game_survivors/src/modules/*.cpp
  ${CMAKE_SOURCE_DIR}/game_survivors/src/resources/*.cpp
  ${CMAKE_SOURCE_DIR}/game_survivors/src/game_state.cpp
  ${CMAKE_SOURCE_DIR}/game_survivors/src/game.cpp
  ${CMAKE_SOURCE_DIR}/game_survivors_tests/src/**.cpp
)

# Test executable
add_executable(game_survivors_tests ${TEST_SRC_FILES})

# Precompile headers
set(PCH_FILE ${CMAKE_SOURCE_DIR}/game_survivors/src/pch.hpp)
target_precompile_headers(game_survivors_tests PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${PCH_FILE}>")
set(PCH_INSTANTIATE_TEMPLATES ON)

link_libs(game_survivors_tests)
link_libs_steam(game_survivors_tests)
target_link_libraries(game_survivors_tests PUBLIC TracyClient)
target_link_libraries(game_survivors_tests PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main)

# copy the box2d.dll next to the game
if(NOT EXISTS "${BOX2D_DLL}")
  message("copying box2d dll next to .exe ${BOX2D_DLL}")
  add_custom_command(TARGET game_survivors_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${BOX2D_DLL}
    $<TARGET_FILE_DIR:game_survivors_tests>
  )
endif()

message("searching for box2d at... ${BOX2D_LIB}")
target_link_libraries(game_survivors_tests PRIVATE ${BOX2D_LIB})

target_include_directories(game_survivors_tests PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${Stb_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/thirdparty/box2d/include
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/src
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/sdk/public
  ${CMAKE_SOURCE_DIR}/thirdparty/glad/include
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/game_survivors/src
  ${CMAKE_SOURCE_DIR}/game_survivors_tests/src
)

if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/arm64-osx/include)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Linux)
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include)
endif()

# Code coverage
if(CMAKE_CXX_COMPILER_ID MATCHES GNU AND CMAKE_BUILD_TYPE MATCHES Debug)
  message("setting code coverage options...")

  # target_compile_options(game_survivors_tests PRIVATE --coverage) # -O0 and -g should already be set
  # target_link_options(game_survivors_tests PRIVATE --coverage)

  # find_program("" lcov)
  # C:/Software/msys64/ucrt64/bin/lcov

  # if(NOT LCOV_PATH)
  # message(FATAL_ERROR "lcov not found!")
  # else()
  # message("lcov found")
  # endif()

  # find_program(GENHTML_PATH genhtml)

  # if(NOT GENHTML_PATH)
  # message(FATAL_ERROR "genhtml not found!")
  # endif()

  # set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage")
  # generatelcovfile
  # add_custom_target(lcov_generate_coverage
  # COMMAND ${LCOV_PATH} --capture --directory .
  # COMMAND ${LCOV_PATH} --remove lcov.info '/usr/*' --output-file lcov.info
  # COMMAND ${LCOV_PATH} --list lcov.info
  # # COMMAND ${GENHTML_PATH} lcov.info --output-directory ${COVERAGE_OUTPUT_DIR}
  # COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated at ${COVERAGE_OUTPUT_DIR}/index.html"
  # WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  # COMMENT "Generating code coverage report..."
  # VERBATIM
  # )

  # # cleanup lcov file
  # add_custom_target(lcov_remove_coverage
  # COMMAND ${LCOV_PATH} --directory . --zerocounters
  # COMMAND ${CMAKE_COMMAND} -E remove lcov.info
  # COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_OUTPUT_DIR}
  # WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  # COMMENT "Cleaning up code coverage data..."
  # VERBATIM
  # )
endif()

# Copy the steam .dll next to the .exe (even if the tests doesnt use it)
if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/steam_api64.dll")
    set(steam_lib ${CMAKE_SOURCE_DIR}/thirdparty/sdk/redistributable_bin/win64)
    message("Copying steam .dll next to .exe")
    set(dst ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET game_survivors_tests POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${steam_lib}/steam_api64.dll
      $<TARGET_FILE_DIR:game_survivors_tests>
    )
  endif()
endif()

# Create symlink for assets only in debug mode.
# This is due to the way I've implemented github
# actions for building release. Github actins will
# copy over the /assets folder in the release mode,
# and would fail if this symlink was created here.
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  message("creating symlink...")
  set(src ${CMAKE_SOURCE_DIR}/game_survivors/assets)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/assets)

  message("creating symlink src... ${src}")
  message("creating symlink dst... ${dst}")

  if(UNIX)
    message("creating symlink... (unix)")
    add_custom_command(
      TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
      DEPENDS ${dst_sym}
      COMMENT "symbolic link resources folder from ${src} => ${dst}"
    )
  else()
    message("creating symlink... (windows)")
    file(TO_NATIVE_PATH "${src}" _srcDir)
    file(TO_NATIVE_PATH "${dst}" _dstDir)

    if(NOT EXISTS ${_dstDir})
      # https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/mklink
      # mklink [[/d] | [/h] | [/j]] <link> <target>
      # <link>	Specifies the name of the symbolic link being created.
      # <target>	Specifies the path (relative or absolute) that the new symbolic link refers to.
      #
      add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND cmd.exe /c mklink /D "${_dstDir}" "${_srcDir}"
        COMMENT "Creating symlink on Windows"
      )
    ELSE()
      message("creating symlink... (already exists)")
    ENDIF()
  ENDIF()
ENDIF()

# If COMMAND specifies an executable target (created by add_executable)
# it will automatically be replaced by the location of the executable
# created at build time
message("adding test to ctest..")
add_test(NAME all_game_survivors_tests COMMAND game_survivors_tests)
