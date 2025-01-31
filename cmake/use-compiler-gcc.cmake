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
  endif()

  # Use lld as the linker
  set(CMAKE_LINKER "C:/Software/msys64/ucrt64/bin/lld.exe")
  set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")
  set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=lld")
  set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=lld")

  # set(CMAKE_LINKER "C:/Software/msys64/ucrt64/bin/ld.bfd.exe")
  # set(CMAKE_LINKER "C:/Software/msys64/ucrt64/bin/ld.exe")
  # set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=bfd") # ld linker
  # set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=bfd") # ld linker
  # set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=bfd") # ld linker
  execute_process(
    COMMAND ${CMAKE_LINKER} --version
    OUTPUT_VARIABLE LINKER_VERSION
  )
  message("Linker: ${LINKER_VERSION}")

  # I have absolutely no idea why,
  # but if I add these lines I dont get "relocation truncated to fit" error messages.
  # I normally dont use the LD linker but do use it with the -pg flags, (which lld doesnt support)
  # so probably dont rely on it for a release build.
  if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    if(LINKER_VERSION MATCHES "GNU ld")
      message("gnu ld linker detected")
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--default-image-base-low")
    endif()
  endif()

  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-report")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic"
  # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
  # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-maybe-uninitialized")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat -Wextra")
  set(LD_FLAGS "${LD_FLAGS} -static")
  message("LD_FLAGS:${LD_FLAGS}")

  # MinGW/GCC specific flags
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Using g++ compiler, enabling -pg for profiling")

    # add_compile_options(-pg)
    # add_link_options(-pg)
    #
    # pacman -S mingw-w64-ucrt-x86_64-python-pip
    # pacman -S mingw-w64-ucrt-x86_64-graphviz
    # pip install gprof2dot
    # gprof --flat-profile -b ohbuoy.exe gmon.out > analysis.txt
    # gprof ohbuoy.exe | gprof2dot | dot -Tpdf -o output.pdf
    #
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -D_DEBUG")
  elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
  elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
  endif()
endif()