# this cmake lists compiles the game_2d with the engine

cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_VERBOSE_MAKEFILE on)

# if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_definitions("-std=c++20")
endif()

project(game_2d VERSION 0.1.0)
message("game_2d: ${CMAKE_SYSTEM_NAME}")
message("game_2d: ${CMAKE_BUILD_TYPE}")
message("game_2d: C: ${CMAKE_C_COMPILER_ID}")
message("game_2d: CXX: ${CMAKE_CXX_COMPILER_ID}")
message("game_2d: current_list_dir: ${CMAKE_CURRENT_LIST_DIR}")
message("game_2d: source_dir: ${CMAKE_SOURCE_DIR}")

# build the engine
include(${CMAKE_SOURCE_DIR}/cmake/imgui.cmake)

# Add VCPKG packages
find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-mixer CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

# find_package(GameNetworkingSockets CONFIG REQUIRED)

# Add source files
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
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  # link windows libs
  target_link_libraries(game_2d PRIVATE SDL2::SDL2 SDL2::SDL2main)
  target_link_libraries(game_2d PRIVATE SDL2::SDL2_mixer)
  target_link_libraries(game_2d PRIVATE opengl32)
  target_link_libraries(game_2d PRIVATE GLEW::GLEW)
  target_link_libraries(game_2d PRIVATE glm::glm)
  target_link_libraries(game_2d PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
  target_link_libraries(game_2d PRIVATE yaml-cpp)

  # create symlink assets for build
  set(source "${CMAKE_SOURCE_DIR}/assets")
  set(destination "${CMAKE_CURRENT_BINARY_DIR}/assets")
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination}
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
  )

  # game networking sockets libs (windows only)
  # OpenSSL::SSL OpenSSL::Crypto
  # protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
  # GameNetworkingSockets::shared GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets GameNetworkingSockets::GameNetworkingSockets_s
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)