#this cmake lists compiles the experimental_networking with the engine

cmake_minimum_required(VERSION 3.0.0)
project(experimental_networking VERSION 0.1.0)

message("experimental_networking: ${CMAKE_SYSTEM_NAME}")
message("experimental_networking: ${CMAKE_BUILD_TYPE}")

# build the engine + bring in Vcpkg
include("${CMAKE_SOURCE_DIR}/engine/cmake/build_info.cmake")

#Add VCPKG packages
foreach(package ${ENGINE_PACKAGES_CONFIG})
  message("${PROJECT_NAME} finding package... ${package}")
  find_package(${package} CONFIG REQUIRED)
endforeach()
foreach(package ${ENGINE_PACKAGES})
  message("${PROJECT_NAME} finding package... ${package}")
  find_package(${package} REQUIRED)
endforeach()

#Add VCPKG header-only
find_path(STB_INCLUDE_DIRS "stb.h")
find_path(BOOST_ASIO_INCLUDE_DIRS "boost/asio.hpp")
find_path(BOOST_CIRCULAR_BUFFER_INCLUDE_DIRS "boost/circular_buffer.hpp")

# add source files
file(GLOB_RECURSE SRC_FILES 
  #engine
  ${ENGINE_SOURCE} 
  #application
  "${CMAKE_SOURCE_DIR}/experimental_networking/src/*.cpp"
)

add_executable(experimental_networking ${SRC_FILES})

# includes
target_include_directories(experimental_networking PRIVATE 
  ${ENGINE_INCLUDES} 
  ${CMAKE_SOURCE_DIR}/experimental_networking/src
)
target_include_directories(experimental_networking PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(experimental_networking PRIVATE ${BOOST_ASIO_INCLUDE_DIRS})
target_include_directories(experimental_networking PRIVATE ${BOOST_CIRCULAR_BUFFER_INCLUDE_DIRS})

# link libs
foreach(library ${ENGINE_LINK_LIBS})
  message("${PROJECT_NAME} linking library... ${library}")
  target_link_libraries(experimental_networking PRIVATE ${library})
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