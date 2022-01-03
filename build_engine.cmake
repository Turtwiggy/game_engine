message("engine_info: ${CMAKE_SYSTEM_NAME}")
message("engine_info: ${CMAKE_BUILD_TYPE}")
message("engine_info: ${CMAKE_CURRENT_LIST_DIR}")

#Source Files
set (ENGINE_SOURCE
    #engine
    ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp

    #imgui
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/backends/imgui_impl_sdl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/imgui_widgets.cpp

    #imguizmo
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo/ImGuizmo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo/ImSequencer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo/ImGradient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo/ImCurveEdit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo/GraphEditor.cpp
)

set (ENGINE_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/backends
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo
    ${STB_INCLUDE_DIRS}
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set (ENGINE_INCLUDES 
        ${ENGINE_INCLUDES} 
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set (ENGINE_INCLUDES 
        ${ENGINE_INCLUDES} 
        ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include
    )
endif()
