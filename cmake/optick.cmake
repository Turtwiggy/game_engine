message("optick: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

if(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  set(OPTICK_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_capi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_core.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_memory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_message.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_miniz.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_serialization.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/optick_server.cpp
  )

  set(OPTICK_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/optick/src/
  )

ELSE()
  set(OPTICK_SOURCE "")
  set(OPTICK_INCLUDES "")
ENDIF()