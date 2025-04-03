if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Bt+")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showIncludes")

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d1reportTime") # debug compiler frontend???
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /d2cgsummary") # debug compiler backend???

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP32")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /fp:precise")

  if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX") # warnings as errors
  endif()

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /external:I ${CMAKE_SOURCE_DIR}/thirdparty")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4275 /wd4251 /bigobj")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /permissive-")

  # Enable Preprocessor output
  # This will generate .i files in the build directory.
  # add_compile_options(/P)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /analyze")

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG /Zi /Od /RTC1")
  endif()

  # set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oi /Ot /Oy /Ob2 /Zi")

  # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /sdl /Oi /Ot /Oy /Ob2 /Zi")
endif()