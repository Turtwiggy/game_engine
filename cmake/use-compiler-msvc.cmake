if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt+")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1reportTime") # debug compiler frontend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d2cgsummary") # debug compiler backend???
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:precise")

  if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX") # warnings as errors
  endif()

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /external:I ${CMAKE_SOURCE_DIR}/thirdparty")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4275 /wd4251 /bigobj")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /permissive-")

  # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /sdl /Oi /Ot /Oy /Ob2 /Zi")
endif()