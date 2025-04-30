cmake_minimum_required(VERSION 3.30.0)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Combine multiple source files into a single translation unit,
# reducing redundant compilation.
# set(CMAKE_UNITY_BUILD ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)
#
project(game_survivors VERSION 0.1.0)
message("game_survivors: CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message("game_survivors: CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message("game_survivors: C: ${CMAKE_C_COMPILER_ID}")
message("game_survivors: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_survivors: CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("game_survivors: CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("game_survivors: CMAKE_LINKER: ${CMAKE_LINKER}")
message("game_survivors: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_survivors: source_dir: ${CMAKE_SOURCE_DIR}")
message("index CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-clang.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-emscripten.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-gcc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-msvc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/steam.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # add_definitions(-DCMAKE_CXX_INCLUDE_WHAT_YOU_USE=include-what-you-use)
  # find_program(IWYU_PATH NAMES include-what-you-use iwyu)
  # set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH};--driver-mode=cl;-Xiwyu;--verbose=3")
endif()

find_packages()

# use steaminpum & launch as steam game
add_compile_definitions(USE_STEAM=1)
set(TRACY_ENABLE 1)
set(TRACY_CALLSTACK 1)

FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.11.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/**/*.cpp"
  ${IMGUI_SOURCE}
  ${CMAKE_SOURCE_DIR}/thirdparty/glad/include/glad/glad.c
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/game_survivors/src/*.cpp
)

# Main executable
if("${CMAKE_BUILD_TYPE}" MATCHES Release AND WIN32)
  # "The WIN32 argument instructs the compiler to build a
  # Windows GUI app, which doesn't show a console."
  add_executable(game_survivors WIN32 ${SRC_FILES} assets/textures/resource.rc)
elseif(WIN32)
  add_executable(game_survivors ${SRC_FILES} assets/textures/resource.rc)
else()
  add_executable(game_survivors ${SRC_FILES})
endif()

# Precompile headers
set(PCH_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp)
target_precompile_headers(game_survivors PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${PCH_FILE}>")
set(PCH_INSTANTIATE_TEMPLATES ON)

link_libs(game_survivors)
link_libs_steam(game_survivors)
target_link_libraries(game_survivors PUBLIC TracyClient)

#
target_include_directories(game_survivors PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${Stb_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/game_survivors/src
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/src
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/sdk/public
  ${CMAKE_SOURCE_DIR}/thirdparty/glad/include
)

# rename executable
if(EMSCRIPTEN)
  # rename as index.html
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "index")
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${LD_FLAGS}")
else()
  set(GAME_EXE_NAME "ohbuoy")
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${GAME_EXE_NAME})
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${LD_FLAGS}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/arm64-osx/include)
elseif(${CMAKE_SYSTEM_NAME} MATCHES Linux)
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include)
endif()

# Create symlink for assets for emscripten
IF(EMSCRIPTEN)
  set(src ${CMAKE_SOURCE_DIR}/game_survivors/assets)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/../assets)
  message("creating symlink src... ${src}")
  message("creating symlink dst... ${dst}")
  add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
    DEPENDS ${dst}
    COMMENT "symbolic link resources folder from ${src} => ${dst}"
  )
ENDIF()

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

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Super cool things")

# set(CPACK_GENERATOR "ZIP") # Generate zip files
include(CPack)