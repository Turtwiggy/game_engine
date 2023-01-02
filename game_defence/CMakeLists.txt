# this cmake lists compiles the game_defence with the engine

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
  add_definitions("-Wformat")
  add_definitions("-O2")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt+")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1reportTime") # debug compiler frontend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d2cgsummary") # debug compiler backend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}/WX /W3 /wd4275 /wd4251")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:fast")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /permissive-")
endif()

project(game_defence VERSION 0.1.0)
message("game_defence: CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message("game_defence: CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message("game_defence: C: ${CMAKE_C_COMPILER_ID}")
message("game_defence: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_defence: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_defence: source_dir: ${CMAKE_SOURCE_DIR}")
message("index CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

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

# Add source files
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)
file(GLOB_RECURSE SRC_FILES
  ${IMGUI_SOURCE}
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/game_defence/src/*.cpp
)

add_executable(game_defence
  ${SRC_FILES}

  # add icon to executable
  ${CMAKE_SOURCE_DIR}/game_defence/assets/textures/resource.rc
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include
  )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(VCPKG_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include
  )
endif()

# includes
target_include_directories(game_defence PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${STB_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/game_defence/src
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/single_include
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui-filebrowser
)

if(EMSCRIPTEN)
  target_link_libraries(game_defence PRIVATE openal)
else()
  # target_link_libraries(game_defence PRIVATE GameNetworkingSockets::shared)
  target_link_libraries(game_defence PRIVATE GLEW::GLEW)
  target_link_libraries(game_defence PRIVATE SDL2::SDL2 SDL2::SDL2main)
  target_link_libraries(game_defence PRIVATE OpenAL::OpenAL)
endif()

target_link_libraries(game_defence PRIVATE glm::glm)
target_link_libraries(game_defence PRIVATE nlohmann_json::nlohmann_json)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  # create symlink assets for build
  set(src ${CMAKE_SOURCE_DIR}/game_defence/assets)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/assets)
  add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
    DEPENDS ${dst}
    COMMENT "symbolic link resources folder from ${src} => ${dst}"
  )
endif()

if(EMSCRIPTEN)
  IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    # create symlink assets for build
    set(src ${CMAKE_SOURCE_DIR}/game_defence/assets)
    set(dst ${CMAKE_CURRENT_BINARY_DIR}/../assets)
    add_custom_command(
      TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
      DEPENDS ${dst}
      COMMENT "symbolic link resources folder from ${src} => ${dst}"
    )
  endif()

  set_target_properties(game_defence PROPERTIES LINK_FLAGS "${LD_FLAGS}")
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)