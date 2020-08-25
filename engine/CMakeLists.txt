#this cmake lists compiles the engine with the engine

cmake_minimum_required(VERSION 3.0.0)
project(engine VERSION 0.1.0)

message("engine: ${CMAKE_SYSTEM_NAME}")
message("engine: ${CMAKE_BUILD_TYPE}")

#Build the Engine
include("${CMAKE_SOURCE_DIR}/engine/cmake/build_info.cmake")

#Add VCPKG packages
foreach(package ${ENGINE_PACKAGES_CONFIG})
  message("finding package... ${package}")
  find_package(${package} CONFIG REQUIRED)
endforeach()
foreach(package ${ENGINE_PACKAGES})
  message("finding package... ${package}")
  find_package(${package} REQUIRED)
endforeach()
find_path(STB_INCLUDE_DIRS "stb.h")

#Add source files
file(GLOB_RECURSE SRC_FILES ${ENGINE_SOURCE} )

add_library(engine ${SRC_FILES})

#Includes
target_include_directories(engine PRIVATE ${ENGINE_INCLUDES} )

#Link Libs
foreach(library ${ENGINE_LINK_LIBS})
  message("linking library... ${library}")
  target_link_libraries(engine PRIVATE ${library})
endforeach()
target_include_directories(engine PRIVATE ${STB_INCLUDE_DIRS})

include(CPack)