cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(game_defence VERSION 0.1.0)
message("game_defence: CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message("game_defence: CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message("game_defence: C: ${CMAKE_C_COMPILER_ID}")
message("game_defence: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_defence: CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("game_defence: CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("game_defence: CMAKE_LINKER: ${CMAKE_LINKER}")
message("game_defence: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_defence: source_dir: ${CMAKE_SOURCE_DIR}")
message("index CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-clang.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-emscripten.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-gcc.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/use-compiler-msvc.cmake)

include(${CMAKE_SOURCE_DIR}/cmake/packages.cmake)
find_packages()

# Add source files
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/optick.cmake)

file(GLOB_RECURSE SRC_FILES
  ${IMGUI_SOURCE}
  ${OPTICK_SOURCE}
  ${CMAKE_SOURCE_DIR}/game_defence/src/*.cpp
)

# Main executable
add_executable(game_defence ${SRC_FILES})

link_libs(game_defence)

target_include_directories(game_defence PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${OPTICK_INCLUDES}
  ${Stb_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/game_defence/src
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/single_include
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui-filebrowser
)

# rename executable
if(EMSCRIPTEN)
  # rename as index.html
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "index")
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${LD_FLAGS}")
else()
  set(GAME_EXE_NAME "solar_warfare")
  set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${GAME_EXE_NAME})
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${LD_FLAGS}")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/arm64-osx/include)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(VCPKG_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include)
endif()

# Create symlink for assets for emscripten
IF(EMSCRIPTEN)
  set(src ${CMAKE_SOURCE_DIR}/game_defence/assets)
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
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  message("creating symlink...")
  set(src ${CMAKE_SOURCE_DIR}/game_defence/assets)
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