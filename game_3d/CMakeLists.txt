# this cmake lists compiles the game_3d with the engine

cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  add_definitions("-std=c++20")
  add_definitions("-Wall")
  add_definitions("-Wextra")
  add_definitions("-Wformat")
  add_definitions("-O2")

  # add_definitions("-Werror")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt+")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1reportTime") # debug compiler frontend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d2cgsummary") # debug compiler backend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}/WX /W3 /wd4275 /wd4251")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:fast")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /permissive-")
endif()

project(game_3d VERSION 0.1.0)
message("game_3d: CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message("game_3d: CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message("game_3d: C: ${CMAKE_C_COMPILER_ID}")
message("game_3d: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_3d: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_3d: source_dir: ${CMAKE_SOURCE_DIR}")
message("index CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message("Target Triplet: ${VCPKG_TARGET_TRIPLET}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  message("setting linux opengl policy...")
  cmake_policy(SET CMP0072 NEW)
  set(OpenGL_GL_PREFERENCE "LEGACY")
  find_package(OpenGL REQUIRED COMPONENTS OpenGL)
endif()

# Add VCPKG packages
if(EMSCRIPTEN)
else()
  # packages not supported or needed by emscripten
  # find_package(GameNetworkingSockets CONFIG REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)
  find_package(GLEW REQUIRED)
  find_package(OpenAL CONFIG REQUIRED)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

# Assimp
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
configure_file("${CMAKE_SOURCE_DIR}/thirdparty/assimp/include/assimp/config.h.in" "${CMAKE_SOURCE_DIR}/thirdparty/assimp/include/assimp/config.h")
configure_file("${CMAKE_SOURCE_DIR}/thirdparty/assimp/contrib/zlib/zconf.h.cmakein" "thirdparty/assimp/contrib/zlib/zconf.h")
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/assimp ${CMAKE_SOURCE_DIR}/assimp)

# Add source files
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/optick.cmake)
file(GLOB_RECURSE SRC_FILES
  ${IMGUI_SOURCE}
  ${OPTICK_SOURCE}
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/game_3d/src/*.cpp
)

add_executable(game_3d
  ${SRC_FILES}

  # add icon to executable
  # ${CMAKE_SOURCE_DIR}/game_3d/assets/textures/resource.rc
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-osx/include
  )
endif()

# includes
target_include_directories(game_3d PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${OPTICK_INCLUDES}
  ${STB_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/game_3d/src
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/single_include
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui-filebrowser
  ${CMAKE_SOURCE_DIR}/thirdparty/assimp/include
)

message("linking libs...")
target_link_libraries(game_3d PRIVATE assimp)
target_link_libraries(game_3d PRIVATE GLEW::GLEW)
target_link_libraries(game_3d PRIVATE glm::glm)
target_link_libraries(game_3d PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(game_3d PRIVATE OpenAL::OpenAL)
target_link_libraries(game_3d PRIVATE SDL2::SDL2 SDL2::SDL2main)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message("linking linux libs...")
  target_link_libraries(game_3d PRIVATE OpenGL::GL)
endif()

# target_link_libraries(game_2d PRIVATE opengl32)
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  # Only do this for debug builds,
  # as release builds should copy the folder over (not symlink)
  IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("creating symlink...")

    # create symlink assets for build
    set(src ${CMAKE_SOURCE_DIR}/game_3d/assets)
    set(dst ${CMAKE_CURRENT_BINARY_DIR}/assets)
    add_custom_command(
      TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
      DEPENDS ${dst}
      COMMENT "symbolic link resources folder from ${src} => ${dst}"
    )
  endif()
endif()

IF(EMSCRIPTEN)
  IF(CMAKE_BUILD_TYPE MATCHES Debug)
    # create symlink assets for build
    set(src ${CMAKE_SOURCE_DIR}/game_3d/assets)
    set(dst ${CMAKE_CURRENT_BINARY_DIR}/../assets)
    add_custom_command(
      TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
      DEPENDS ${dst}
      COMMENT "symbolic link resources folder from ${src} => ${dst}"
    )
  endif()

  set_target_properties(game_3d PROPERTIES LINK_FLAGS "${LD_FLAGS}")
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)