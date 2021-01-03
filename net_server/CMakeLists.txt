# this cmake lists compiles the net_server with the engine

cmake_minimum_required(VERSION 3.0.0)
project(net_server VERSION 0.1.0)

message("net_server: ${CMAKE_SYSTEM_NAME}")
message("net_server: ${CMAKE_BUILD_TYPE}")

# build the engine + bring in vcpkg
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
# Add VCPKG header-only librarys
find_path(STB_INCLUDE_DIRS "stb.h")
find_path(BOOST_ASIO_INCLUDE_DIRS "boost/asio.hpp")

# add source files
file(GLOB_RECURSE SRC_FILES 
  ${ENGINE_SOURCE} 
  "${CMAKE_SOURCE_DIR}/net_server/src/*.cpp"
)

add_executable(net_server ${SRC_FILES})

# engine and project includes
target_include_directories(net_server PRIVATE 
  ${ENGINE_INCLUDES} 
  ${CMAKE_SOURCE_DIR}/net_server/src
)
# vcpkg third party includes
target_include_directories(net_server PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(net_server PRIVATE ${BOOST_ASIO_INCLUDE_DIRS})

# link libs
foreach(library ${ENGINE_LINK_LIBS})
  # message("${PROJECT_NAME} linking library... ${library}")
  target_link_libraries(net_server PRIVATE ${library})
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