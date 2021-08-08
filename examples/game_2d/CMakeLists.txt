#this cmake lists compiles the game_2d with the engine

cmake_minimum_required(VERSION 3.0.0)
project(game_2d VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("game_2d: ${CMAKE_SYSTEM_NAME}")
message("game_2d: ${CMAKE_BUILD_TYPE}")

# build the engine + bring in Vcpkg
include("${CMAKE_SOURCE_DIR}/engine/cmake/build_info.cmake")

# Add VCPKG packages
foreach(package ${ENGINE_PACKAGES_CONFIG})
  message("${PROJECT_NAME} finding package... ${package}")
  find_package(${package} CONFIG REQUIRED)
endforeach()
foreach(package ${ENGINE_PACKAGES})
  message("${PROJECT_NAME} finding package... ${package}")
  find_package(${package} REQUIRED)
endforeach()

# Add VCPKG header-only
find_path(STB_INCLUDE_DIRS "stb.h")

# Add source files
file(GLOB_RECURSE SRC_FILES 
  ${ENGINE_SOURCE},
  "${CMAKE_SOURCE_DIR}/examples/game_2d/src/*.cpp"
)

add_executable(game_2d ${SRC_FILES})

# includes
target_include_directories(game_2d PRIVATE 
  ${ENGINE_INCLUDES} 
  ${CMAKE_SOURCE_DIR}/examples/game_2d/src
)

# link libs
foreach(library ${ENGINE_LINK_LIBS})
  message("${PROJECT_NAME} linking library... ${library}")
  target_link_libraries(game_2d PRIVATE ${library})
endforeach()

# create symlink assets for build
IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set (source "${CMAKE_SOURCE_DIR}/assets")
  set (destination "${CMAKE_CURRENT_BINARY_DIR}/assets")
  add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${destination} 
    DEPENDS ${destination}
    COMMENT "symbolic link resources folder from ${source} => ${destination}"
  )
endif()

include(CPack)