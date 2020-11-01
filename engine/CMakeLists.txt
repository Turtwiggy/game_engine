# this cmake lists compiles fightingengine_tests

cmake_minimum_required(VERSION 3.0.0)
project(fightingengine_tests VERSION 0.1.0)

message("fightingengine_tests: ${CMAKE_SYSTEM_NAME}")
message("fightingengine_tests: ${CMAKE_BUILD_TYPE}")

# build the engine + bring in Vcpkg
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

#Add VCPKG header-only
find_path(STB_INCLUDE_DIRS "stb.h")

#Add Testing
enable_testing()
find_package(GTest CONFIG REQUIRED)

# add source files
file(GLOB_RECURSE SRC_FILES 
  ${ENGINE_SOURCE}
  "${CMAKE_SOURCE_DIR}/engine/test/*.cpp"
)

add_executable(fightingengine_tests ${SRC_FILES} )

#Includes
target_include_directories(fightingengine_tests PRIVATE 
  ${ENGINE_INCLUDES} 
  ${STB_INCLUDE_DIRS}
)

#Link Libs
foreach(library ${ENGINE_LINK_LIBS})
  message("linking library... ${library}")
  target_link_libraries(fightingengine_tests PRIVATE ${library})
endforeach()
#Link Testing Libs
target_link_libraries(fightingengine_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

include(CPack)