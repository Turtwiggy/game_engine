message("engine_info: ${CMAKE_SYSTEM_NAME}")
message("engine_info: ${CMAKE_BUILD_TYPE}")
message("engine_info: ${CMAKE_CURRENT_LIST_DIR}")

#VCPKG packages
# set (ENGINE_PACKAGES_CONFIG
#     EnTT SDL2 sdl2-mixer
# )
# set (ENGINE_PACKAGES
#     OpenGL GLEW
# )

#Source Files
set (ENGINE_SOURCE
    #engine
    ${CMAKE_CURRENT_LIST_DIR}/engine/*.cpp

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
    ${CMAKE_CURRENT_LIST_DIR}/engine
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/backends
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imguizmo
    ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glm
)

# if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
#     #Vcpkg windows includes
#     ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-windows/include

#     #Vcpkg windows links
#     set (ENGINE_LINK_LIBS
#         SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_mixer
#         EnTT::EnTT
#         opengl32
#         GLEW::GLEW
#         # game networking sockets libs (windows only)
#         # OpenSSL::SSL OpenSSL::Crypto
#         # protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
#         # GameNetworkingSockets::shared GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets GameNetworkingSockets::GameNetworkingSockets_s
#     )

# elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
#     #Vcpkg linux includes
#     set (ENGINE_INCLUDES
#         ${CMAKE_CURRENT_LIST_DIR}/engine/src
#         ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui
#         ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui/examples
#         ${CMAKE_CURRENT_LIST_DIR}/thirdparty/vcpkg/installed/x64-linux/include

#         ${STB_INCLUDE_DIRS}
#     )
#     #Vcpkg linux links
#     set (ENGINE_LINK_LIBS
#         SDL2::SDL2main SDL2::SDL2-static
#         EnTT::EnTT
#         GL
#         GLEW
#     )
# endif()