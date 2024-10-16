if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  message("setting cxx flags for gnu...")

  if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    message("sledgehammer path changes...")

    # set(CMAKE_C_COMPILER "C:/Software/msys64/ucrt64/bin/gcc.exe")
    # set(CMAKE_CXX_COMPILER "C:/Software/msys64/ucrt64/bin/g++.exe")
    # set(CMAKE_AR "C:/Software/msys64/ucrt64/bin/ar.exe")
    # set(CMAKE_C_FLAGS_INIT "-L C:/Software/msys64/ucrt64/lib")
    # set(CMAKE_CXX_FLAGS_INIT "-L C:/Software/msys64/ucrt64/lib")
    # set(CMAKE_INCLUDE_PATH "C:/Software/msys64/ucrt64/include")
    # set(CMAKE_LIBRARY_PATH "C:/Software/msys64/ucrt64/lib")
    set(CMAKE_LINKER "C:/Software/msys64/ucrt64/bin/lld.exe")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat -Wextra")

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-report")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic"
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
  # set(LD_FLAGS "${LD_FLAGS} -static -static-libgcc -static-libstdc++")
  set(LD_FLAGS "${LD_FLAGS} -static")
  message("LD_FLAGS:${LD_FLAGS}")

  # Use lld as the linker
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
  set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld")
  set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=lld")

  # MinGW/GCC specific flags
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-inline -D_DEBUG")
  elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
  elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
  endif()
endif()