# Note: CMAKE_SYSTEM_NAME should be: Emscripten NOT windows
if(EMSCRIPTEN)
  message("Compiling for emscripten (toolkit set in cmake-kits.json)")

  # https://emscripten.org/docs/tools_reference/emcc.html
  # https://emscripten.org/docs/porting/files/packaging_files.html
  set(EMS "-sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sSDL2_MIXER_FORMATS=wav,mp3 -sDISABLE_EXCEPTION_CATCHING=1 ")

  # set(EMS "${EMS} -sUSE_PTHREADS=1")
  # set(LD_FLAGS "${LD_FLAGS} -sPTHREAD_POOL_SIZE=4")
  set(LD_FLAGS "-sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sNO_EXIT_RUNTIME=0 -sASSERTIONS=1")
  set(LD_FLAGS "${LD_FLAGS} -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2")
  set(LD_FLAGS "${LD_FLAGS} --preload-file assets --bind")
  set(LD_FLAGS "${LD_FLAGS} --shell-file  assets/emscripten/shell_minimal.html")
  set(LD_FLAGS "${LD_FLAGS} ${EMS}")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -stdlib=libc++ -O2 ${EMS}")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  message("index VCPKG_CHAINLOAD_TOOLCHAIN_FILE: ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
  message("index CMAKE_EXECUTABLE_SUFFIX: ${CMAKE_EXECUTABLE_SUFFIX}")
endif()