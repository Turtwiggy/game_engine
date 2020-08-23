workspace "FightingGameWorkspace"
    architecture "x86_64"
    startproject "game_3d"
    characterset "MBCS"

    configurations
    {
        "Debug",
        "Release"
    }

    flags
    {
        "MultiProcessorCompile",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["ImGui"] = "thirdparty/imgui"
IncludeDir["ImGui2"] = "thirdparty/imgui/examples"
IncludeDir["ggpo"]  = "thirdparty/ggpo/src/include"
IncludeDir["GameNetworkingSockets"] = "thirdparty/gamenetworkingsockets/include"

LibDir = {}
DllDir = {}

-- VCPKG packages
if os.host() ==  "windows" then
    printf("building windows!")
    IncludeDir["vcpkg"] = "thirdparty/vcpkg/installed/x64-windows/include"
    LibDir["vcpkg"] = "thirdparty/vcpkg/installed/x64-windows/lib/"
    DllDir["vcpkg"] = "thirdparty/vcpkg/installed/x64-windows/bin/"
end

if os.host() == "linux" then
    printf("building linux!")
    IncludeDir["vcpkg"] = "thirdparty/vcpkg/installed/x64-linux/include"
    LibDir["vcpkg"] = "thirdparty/vcpkg/installed/x64-linux/lib/"
    DllDir["vcpkg"] = "thirdparty/vcpkg/installed/x64-linux/bin/"
end

ImguiSourceFiles = {}
ImguiSourceFiles["imgui1"] = "thirdparty/imgui/imgui_widgets.cpp"
ImguiSourceFiles["imgui2"] = "thirdparty/imgui/examples/imgui_impl_sdl.cpp"
ImguiSourceFiles["imgui3"] = "thirdparty/imgui/examples/imgui_impl_opengl3.cpp"
ImguiSourceFiles["imgui4"] = "thirdparty/imgui/imgui.cpp"
ImguiSourceFiles["imgui5"] = "thirdparty/imgui/imgui_draw.cpp"
ImguiSourceFiles["imgui6"] = "thirdparty/imgui/imgui_demo.cpp"

project "fighting_engine"
    location "fighting_engine"
	kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
        "%{ImguiSourceFiles.imgui1}",
        "%{ImguiSourceFiles.imgui2}",
        "%{ImguiSourceFiles.imgui3}",
        "%{ImguiSourceFiles.imgui4}",
        "%{ImguiSourceFiles.imgui5}",
        "%{ImguiSourceFiles.imgui6}",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui2}",
        "%{IncludeDir.ggpo}",
        "%{IncludeDir.vcpkg}",
    }

    libdirs 
    {
        "%{LibDir.vcpkg}"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "IMGUI_IMPL_OPENGL_LOADER_GLEW"
    }
    
    filter "system:windows"
        systemversion "latest"
        
        --(win libs) do not statically link
        links 
        { 
            "opengl32",
            "SDL2",
            "glew32",
            "assimp-vc142-mt",
            "fmt"
        }

        defines
        {
            "__WIN32__",
        }
    
    filter "system:linux"

        links { "dl", "GL", "pthread", "X11" }

        buildoptions 
        {
            "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-O2", "-s"
        }

    filter "configurations:Debug"
        defines {"ENGINE_DEBUG", "DEBUG"}
        runtime "Debug"
        symbols "on"
        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MDd"
            }

	filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MD"
            }

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }

    filter{}

project "game_3d"
    location "game_3d"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

    -- pchheader "spkpch.h"
    -- pchsource "SparkEngine/src/spkpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",

        -- build engine directly
        "fighting_engine/src/**.cpp",

        -- build imgui
        "%{ImguiSourceFiles.imgui1}",
        "%{ImguiSourceFiles.imgui2}",
        "%{ImguiSourceFiles.imgui3}",
        "%{ImguiSourceFiles.imgui4}",
        "%{ImguiSourceFiles.imgui5}",
        "%{ImguiSourceFiles.imgui6}",
    }

    includedirs
    {
        "%{prj.name}/src",
        "fighting_engine/src",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui2}",
        "%{IncludeDir.vcpkg}",
    }

    libdirs 
    {
        "%{LibDir.vcpkg}"
        -- "./dlls/"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "IMGUI_IMPL_OPENGL_LOADER_GLEW"
    }

    filter "system:windows"
        systemversion "latest"
        
        --(win libs) do not statically link
        links 
        { 
            "opengl32",
            "SDL2",
            "glew32",
            "assimp-vc142-mt",
            "fmt"
        }

        defines
        {
            "__WIN32__",
        }
    
    filter "system:linux"

        links { "dl", "GL", "pthread", "X11" }

        buildoptions 
        {
            "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-O2", "-s"
        }

    filter "configurations:Debug"
        defines {"GAME_DEBUG", "DEBUG"}
        runtime "Debug"
        symbols "on"
        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MDd"
            }
            
	filter "configurations:Release"
        defines "GAME_RELEASE"
        runtime "Release"
        optimize "on"

        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MD"
            }

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }



    filter{}

    postbuildcommands -- copy resources after build
    {
    	("{COPY} $(SolutionDir)/assets $(TargetDir)/assets")
    }

    postbuildcommands -- copy dlls next to exe
    {
        ("{COPY} $(SolutionDir)/" ..  DllDir["vcpkg"] .. " $(TargetDir)/")
    }
    
project "game_2d"
    location "game_2d"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

    -- pchheader "spkpch.h"
    -- pchsource "SparkEngine/src/spkpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",

        -- build engine directly
        "fighting_engine/src/**.cpp",

        -- build imgui
        "%{ImguiSourceFiles.imgui1}",
        "%{ImguiSourceFiles.imgui2}",
        "%{ImguiSourceFiles.imgui3}",
        "%{ImguiSourceFiles.imgui4}",
        "%{ImguiSourceFiles.imgui5}",
        "%{ImguiSourceFiles.imgui6}",
    }

    includedirs
    {
        "%{prj.name}/src",
        "fighting_engine/src",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui2}",
        "%{IncludeDir.vcpkg}",
    }

    libdirs 
    {
        -- "./dlls/"
    }

    links
    {
        -- "GameNetworkingSockets",
        "opengl32"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "IMGUI_IMPL_OPENGL_LOADER_GLEW"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "__WIN32__",
        }

    filter "configurations:Debug"
        defines {"GAME_DEBUG", "DEBUG"}
        runtime "Debug"
        symbols "on"
        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MDd"
            }

	filter "configurations:Release"
        defines "GAME_RELEASE"
        runtime "Release"
        optimize "on"
        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MD"
            }

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }

    -- configuration "gmake2"
    --     buildoptions 
    --     {
    --         "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-O2", "-s"
    --     }
    --     links { }

    filter{}

    postbuildcommands -- copy resources after build
    {
    	("{COPY} $(SolutionDir)/game_2d/assets $(TargetDir)/assets")
    }

project "raytracing_oneweekend"
    location "raytracing_oneweekend"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
        "fighting_engine/src"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
    }

    links 
    { 
        "fighting_engine"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "__WIN32__",
        }

    filter "configurations:Debug"
        defines {"ENGINE_DEBUG", "DEBUG"}
        runtime "Debug"
        symbols "off"
        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MDd"
            }

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"
        configuration "vs2019"
            buildoptions 
            {
                "/bigobj" , "/permissive-", "/MD"
            }

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }

    filter{}