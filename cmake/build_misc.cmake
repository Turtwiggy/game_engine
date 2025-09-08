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
      COMMENT "symbolic link resources folder from ${src} => ${dst}"
    )
  else()
    message("creating symlink... (windows)")
    file(TO_NATIVE_PATH "${src}" _srcDir)
    file(TO_NATIVE_PATH "${dst}" _dstDir)

    if(EXISTS ${_dstDir})
      message("creating symlink... (already exists)")
      return()
    endif()

    # https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/mklink
    # mklink [[/d] | [/h] | [/j]] <link> <target>
    # <link>	Specifies the name of the symbolic link being created.
    # <target>	Specifies the path (relative or absolute) that the new symbolic link refers to.
    #
    execute_process(
      COMMAND cmd.exe /c mklink /D "${_dstDir}" "${_srcDir}"
      RESULT_VARIABLE result
      ERROR_VARIABLE error
    )

    if(NOT result EQUAL 0)
      message(WARNING "Failed to create symlink: ${error}")
    endif()
  ENDIF()
endfunction()

function(copy_file_next_to_exe project file_src file_dst)
  if(NOT EXISTS ${file_src})
    message(WARNING "File not found: ${file_src}")
    return()
  endif()

  if(EXISTS ${file_dst})
    message("File exists at dst: ${file_dst}")
    return()
  endif()

  message("copying... ${file_src} => ${file_dst}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file_src} ${file_dst}
    RESULT_VARIABLE result
    ERROR_VARIABLE error
  )

  if(NOT result EQUAL 0)
    message(WARNING "Failed to copy file: ${error}")
  else()
    message("File copied.")
  endif()
endfunction()