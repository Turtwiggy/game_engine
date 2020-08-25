#this cmake lists compiles the game_3d with the engine

cmake_minimum_required(VERSION 3.0.0)
project(game_3d VERSION 0.1.0)

message("game_3d: ${CMAKE_SYSTEM_NAME}")
message("game_3d: ${CMAKE_BUILD_TYPE}")

# build the engine + bring in Vcpkg
include("${CMAKE_SOURCE_DIR}/engine/cmake/build_info.cmake")

# add Vcpkg packages
foreach(package ${ENGINE_PACKAGES_CONFIG})
  message("${PROJECT_NAME} finding package... ${package}")
  find_package(${package} CONFIG REQUIRED)
endforeach()
foreach(package ${ENGINE_PACKAGES})
  message("${PROJECT_NAME} finding package... ${package}")
  find_package(${package} REQUIRED)
endforeach()
find_path(STB_INCLUDE_DIRS "stb.h")

# add source files
file(GLOB_RECURSE SRC_FILES 

  #engine source
  ${ENGINE_SOURCE} 

  #game source
  "${CMAKE_SOURCE_DIR}/game_3d/src/*.cpp"
)

add_executable(game_3d ${SRC_FILES})

# includes
target_include_directories(game_3d PRIVATE 

  #engine includes
  ${ENGINE_INCLUDES} 

  #game includes
  ${CMAKE_SOURCE_DIR}/game_3d/src

)

# link libs
foreach(library ${ENGINE_LINK_LIBS})
  message("${PROJECT_NAME} linking library... ${library}")
  target_link_libraries(game_3d PRIVATE ${library})
endforeach()
target_include_directories(game_3d PRIVATE ${STB_INCLUDE_DIRS})

#create symlink assets for build
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