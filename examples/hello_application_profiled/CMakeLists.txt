#this cmake lists compiles the hello_application_profiled with the engine

cmake_minimum_required(VERSION 3.0.0)
project(hello_application_profiled VERSION 0.1.0)

message("hello_application_profiled: ${CMAKE_SYSTEM_NAME}")
message("hello_application_profiled: ${CMAKE_BUILD_TYPE}")

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

# add source files
file(GLOB_RECURSE SRC_FILES 
  ${ENGINE_SOURCE} 
  "${CMAKE_SOURCE_DIR}/examples/hello_application_profiled/src/*.cpp"
)

add_executable(hello_application_profiled ${SRC_FILES})

# includes
target_include_directories(hello_application_profiled PRIVATE 
  ${ENGINE_INCLUDES} 
  ${CMAKE_SOURCE_DIR}/examples/hello_application_profiled/src
)
target_include_directories(hello_application_profiled PRIVATE ${STB_INCLUDE_DIRS})

# link libs
foreach(library ${ENGINE_LINK_LIBS})
  message("${PROJECT_NAME} linking library... ${library}")
  target_link_libraries(hello_application_profiled PRIVATE ${library})
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