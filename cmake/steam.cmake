function(link_libs_steam project)
  if(${CMAKE_SYSTEM_NAME} MATCHES Windows)
    # Link steam_api[64].lib
    set(steam_dll_dst ${CMAKE_CURRENT_BINARY_DIR}/steam_api64.dll)
    set(steam_dll_src ${CMAKE_SOURCE_DIR}/thirdparty/sdk/redistributable_bin/win64/steam_api64.dll)
    set(steam_lib_src ${CMAKE_SOURCE_DIR}/thirdparty/sdk/redistributable_bin/win64/steam_api64.lib)
    target_link_libraries(${project} PRIVATE ${steam_lib_src})
  endif()

  if(${CMAKE_SYSTEM_NAME} MATCHES Darwin)
    # Link steam_api[64].lib
    set(steam_dll_dst ${CMAKE_CURRENT_BINARY_DIR}/libsteam_api.dylib)
    set(steam_dll_src ${CMAKE_SOURCE_DIR}/thirdparty/sdk/redistributable_bin/osx/libsteam_api.dylib)
    set(steam_lib_src ${CMAKE_SOURCE_DIR}/thirdparty/sdk/redistributable_bin/osx/libsteam_api.dylib)
    target_link_libraries(${project} PRIVATE ${steam_lib_src})
  endif()
endfunction()

# Copy the .dll next to the .exe
# if(NOT EXISTS ${steam_dll_dst})
# message("Copying steam .dll next to .exe")
# add_custom_command(TARGET game_survivors_tests POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy ${steam_dll_src} ${steam_dll_dst}
# )
# endif()