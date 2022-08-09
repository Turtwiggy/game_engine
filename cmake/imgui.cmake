message("imgui: CMAKE_CURRENT_LIST_DIR: ${CMAKE_CURRENT_LIST_DIR}")

set(IMGUI_SOURCE
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/backends/imgui_impl_sdl.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/backends/imgui_impl_opengl3.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/imgui.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/imgui_draw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/imgui_demo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui/misc/cpp/imgui_stdlib.cpp
)

set(IMGUI_INCLUDES
  ${CMAKE_CURRENT_LIST_DIR}/../thirdparty/imgui
)
