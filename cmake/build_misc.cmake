function(create_symlinks project)
  #
  message("creating symlink...")
  set(src ${CMAKE_SOURCE_DIR}/game_survivors/assets)
  set(dst ${CMAKE_CURRENT_BINARY_DIR}/assets)
  message("creating symlink src... ${src}")
  message("creating symlink dst... ${dst}")

  if(UNIX)
    message("creating symlink... (unix)")
    add_custom_command(
      TARGET ${PROJECT_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
      DEPENDS ${dst_sym}
      COMMENT "symbolic link resources folder from ${src} => ${dst}"
    )
  else()
    message("creating symlink... (windows)")
    file(TO_NATIVE_PATH "${src}" _srcDir)
    file(TO_NATIVE_PATH "${dst}" _dstDir)

    if(NOT EXISTS ${_dstDir})
      # https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/mklink
      # mklink [[/d] | [/h] | [/j]] <link> <target>
      # <link>	Specifies the name of the symbolic link being created.
      # <target>	Specifies the path (relative or absolute) that the new symbolic link refers to.
      #
      add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND cmd.exe /c mklink /D "${_dstDir}" "${_srcDir}"
        COMMENT "Creating symlink on Windows"
      )
    ELSE()
      message("creating symlink... (already exists)")
    ENDIF()
  ENDIF()
endfunction()

function(copy_file_next_to_exe project file)
  if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/steam_api64.dll")
    set(steam_lib ${CMAKE_SOURCE_DIR}/thirdparty/sdk/redistributable_bin/win64)
    message("copying file... ${file}")
    add_custom_command(
      TARGET ${project} PRE_LINK
      COMMAND ${CMAKE_COMMAND} -E copy_if_different file $<TARGET_FILE_DIR:${project}>
    )
  endif()
endfunction()