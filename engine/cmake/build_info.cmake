set (CMAKE_CXX_STANDARD 17)
set (CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


#VCPKG packages
set (ENGINE_PACKAGES_CONFIG
    SDL2 EnTT Bullet spdlog glm assimp
)
set (ENGINE_PACKAGES
    OpenGL GLEW
)

#Source Files
set (ENGINE_SOURCE
    #engine    
    "${CMAKE_SOURCE_DIR}/engine/src/*.cpp"

    #imgui
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples/imgui_impl_sdl.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples/imgui_impl_opengl3.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp"
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    #Vcpkg windows includes
    set (ENGINE_INCLUDES
        ${CMAKE_SOURCE_DIR}/engine/src
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples
        ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-windows/include
    )
    #Vcpkg windows links
    set (ENGINE_LINK_LIBS
        EnTT::EnTT
        LinearMath Bullet3Common BulletDynamics BulletSoftBody
        spdlog::spdlog spdlog::spdlog_header_only
        glm
        GLEW::GLEW
        SDL2::SDL2 SDL2::SDL2main
        opengl32
        assimp::assimp
    )

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    #Vcpkg linux includes
    set (ENGINE_INCLUDES
        ${CMAKE_SOURCE_DIR}/engine/src
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui
        ${CMAKE_SOURCE_DIR}/thirdparty/imgui/examples
        ${CMAKE_SOURCE_DIR}/thirdparty/vcpkg/installed/x64-linux/include
    )
    #Vcpkg linux links
    set (ENGINE_LINK_LIBS
        EnTT::EnTT
        LinearMath Bullet3Common BulletDynamics BulletSoftBody
        spdlog::spdlog spdlog::spdlog_header_only
        glm
        GLEW
        SDL2::SDL2main SDL2::SDL2-static
        GL
        #missing assimp here
    )
endif()  



