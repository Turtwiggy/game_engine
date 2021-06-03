set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

message("engine_info: ${CMAKE_SYSTEM_NAME}")
message("engine_info: ${CMAKE_BUILD_TYPE}")

#VCPKG packages
set (ENGINE_PACKAGES_CONFIG
    SDL2 glm assimp protobuf OpenAL SndFile GameNetworkingSockets
)
set (ENGINE_PACKAGES
    OpenSSL OpenGL GLEW
)

#Source Files
set (ENGINE_SOURCE
    #engine    
    ${CMAKE_SOURCE_DIR}/engine/src/*.cpp

    #imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_sdl.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp

    #imguizmo
    ${CMAKE_SOURCE_DIR}/thirdparty/imguizmo/ImGuizmo.cpp
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    #Vcpkg windows includes
    set (ENGINE_INCLUDES
        ${CMAKE_SOURCE_DIR}/engine/src
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends
        ${CMAKE_SOURCE_DIR}/thirdparty/imguizmo
        ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-windows/include

        ${STB_INCLUDE_DIRS}
    )
    #Vcpkg windows links
    set (ENGINE_LINK_LIBS
        SDL2::SDL2 SDL2::SDL2main
        glm
        assimp::assimp
        opengl32
        GLEW::GLEW
        OpenAL::OpenAL
        SndFile::sndfile
        # game networking sockets libs (windows only)
        OpenSSL::SSL OpenSSL::Crypto
        protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
        GameNetworkingSockets::shared GameNetworkingSockets::static GameNetworkingSockets::GameNetworkingSockets GameNetworkingSockets::GameNetworkingSockets_s)

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    #Vcpkg linux includes
    set (ENGINE_INCLUDES
        ${CMAKE_SOURCE_DIR}/engine/src
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples
        ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-linux/include

        ${STB_INCLUDE_DIRS}
    )
    #Vcpkg linux links
    set (ENGINE_LINK_LIBS
        SDL2::SDL2main SDL2::SDL2-static
        glm
        assimp::assimp
        GL
        GLEW
    )
endif()  



