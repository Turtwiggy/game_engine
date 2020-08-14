workspace "FightingGameWorkspace"
    architecture "x86_64"
    startproject "fighting_game"
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
    staticruntime "on"

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
            "opengl32"
        }

        defines
        {
            "__WIN32__",
        }

	filter "system:linux"
        links { "dl", "GL", "pthread", "X11" }
 
    filter "configurations:Debug"
        defines {"ENGINE_DEBUG", "DEBUG"}
        runtime "Debug"
        symbols "on"
        buildoptions {"/bigobj" , "/permissive-", "/MDd"}

	filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"
        buildoptions {"/bigobj" , "/permissive-", "/MD"}

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }

    filter{}

project "fighting_game"
    location "fighting_game"
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
        "%{IncludeDir.ImGui2}"
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
        buildoptions {"/bigobj" , "/permissive-", "/MDd"}

	filter "configurations:Release"
        defines "GAME_RELEASE"
        runtime "Release"
        optimize "on"
        buildoptions {"/bigobj" , "/permissive-", "/MD"}

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
    	("{COPY} $(SolutionDir)/assets $(TargetDir)/assets")
    }
    
project "game_breakout"
    location "game_breakout"
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
        "%{IncludeDir.ImGui2}"
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
        buildoptions {"/bigobj" , "/permissive-", "/MDd"}

	filter "configurations:Release"
        defines "GAME_RELEASE"
        runtime "Release"
        optimize "on"
        buildoptions {"/bigobj" , "/permissive-", "/MD"}

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
    	("{COPY} $(SolutionDir)/game_breakout/assets $(TargetDir)/assets")
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
        buildoptions {"/bigobj" , "/permissive-", "/MDd"}

    filter "configurations:Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"
        buildoptions {"/bigobj" , "/permissive-", "/MD"}

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }

    filter{}