# this cmake lists compiles the game_2d with the engine

cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EMSCRIPTEN)
  message("Compiling for emscripten (toolkit set in cmake-kits.json)")

  # https://emscripten.org/docs/tools_reference/emcc.html
  # add_compile_definitions("-s ALLOW_MEMORY_GROWTH=1")
  # add_compile_definitions(" --shell-file shell_minimal.html")
  set(USE_FLAGS "-s USE_SDL=2 -s DISABLE_EXCEPTION_CATCHING=1 -s WASM=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shell-file ${CMAKE_SOURCE_DIR}/assets/emscripten/shell_minimal.html")
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

project(game_2d VERSION 0.1.0)
message("game_2d: CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}")
message("game_2d: CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
message("game_2d: C: ${CMAKE_C_COMPILER_ID}")
message("game_2d: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_2d: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_2d: source_dir: ${CMAKE_SOURCE_DIR}")
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
)

add_executable(game_2d
  ${SRC_FILES}

  # add icon to executable
  ${CMAKE_SOURCE_DIR}/assets/textures/resource.rc
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
target_include_directories(game_2d PRIVATE
  ${IMGUI_INCLUDES}
  ${VCPKG_INCLUDES}
  ${STB_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/thirdparty/entt/single_include
  ${CMAKE_SOURCE_DIR}/thirdparty/magic_enum
  ${CMAKE_SOURCE_DIR}/thirdparty/imgui-filebrowser
)

if(EMSCRIPTEN)
  target_link_libraries(game_2d PRIVATE openal)
else()
  # target_link_libraries(game_2d PRIVATE GameNetworkingSockets::shared)
  target_link_libraries(game_2d PRIVATE GLEW::GLEW)
  target_link_libraries(game_2d PRIVATE SDL2::SDL2 SDL2::SDL2main)
  target_link_libraries(game_2d PRIVATE OpenAL::OpenAL)
endif()

target_link_libraries(game_2d PRIVATE glm::glm)
target_link_libraries(game_2d PRIVATE nlohmann_json::nlohmann_json)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  # create symlink assets for build
  set(src ${CMAKE_SOURCE_DIR}/assets)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/assets)
  add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
    DEPENDS ${dst}
    COMMENT "symbolic link resources folder from ${src} => ${dst}"
  )

  # (server) copy files
  # set(server_src ${CMAKE_CURRENT_BINARY_DIR}/)
  # set(server_dst ${CMAKE_CURREdNT_BINARY_DIR}/../server)
  # add_custom_command(
  # TARGET ${PROJECT_NAME} POST_BUILD
  # COMMAND ${CMAKE_COMMAND} -E copy_directory ${server_src} ${server_dst}
  # COMMENT "copy files from ${server_src} => ${server_dst}"
  # )

  # (server) make symlink
  # set(server_asset_src ${CMAKE_SOURCE_DIR}/assets)
  # set(server_asset_dst "${CMAKE_CURRENT_BINARY_DIR}/../server/assets")
  # add_custom_command(
  # TARGET ${PROJECT_NAME} POST_BUILD
  # COMMAND ${CMAKE_COMMAND} -E create_symlink ${server_asset_src} ${server_asset_dst}
  # DEPENDS ${server_asset_dst}
  # COMMENT "symbolic link resources folder from ${server_asset_src} => ${server_asset_dst}"
  # )
endif()

if(EMSCRIPTEN)
  # create symlink assets for build
  set(src ${CMAKE_SOURCE_DIR}/assets)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/../assets)
  add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
    DEPENDS ${dst}
    COMMENT "symbolic link resources folder from ${src} => ${dst}"
  )

  # See documentation for more details: https://emscripten.org/docs/porting/files/packaging_files.html
  set_target_properties(game_2d PROPERTIES LINK_FLAGS "-s DEMANGLE_SUPPORT=1 --preload-file assets --bind")
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)