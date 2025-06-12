# Copy box2d dll to game
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
  set(BOX2D_LIB ${CMAKE_SOURCE_DIR}/thirdparty/box2d/build/src/Debug/box2dd.lib)
  set(BOX2D_DLL ${CMAKE_SOURCE_DIR}/thirdparty/box2d/build/bin/Debug/box2dd.dll)
  set(BOX2D_PDB ${CMAKE_SOURCE_DIR}/thirdparty/box2d/build/bin/Debug/box2dd.pdb)
else()
  set(BOX2D_LIB ${CMAKE_SOURCE_DIR}/thirdparty/box2d/build/src/Release/box2d.lib)
  set(BOX2D_DLL ${CMAKE_SOURCE_DIR}/thirdparty/box2d/build/bin/Release/box2d.dll)
endif()

message("searching for .lib box2d at... ${BOX2D_LIB}")
message("searching for .dll box2d at... ${BOX2D_DLL}")
