workspace "FightingGameWorkspace"
    architecture "x86_64"
    startproject "FightingGameMain"
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
IncludeDir["ImGui"] = "deps/imgui"
IncludeDir["ImGui2"] = "deps/imgui/examples"
IncludeDir["ggpo"]  = "deps/ggpo/src/include"
IncludeDir["GameNetworkingSockets"] = "deps/gamenetworkingsockets/include"
 
ImguiSourceFiles = {}
ImguiSourceFiles["imgui1"] = "deps/imgui/imgui_widgets.cpp"
ImguiSourceFiles["imgui2"] = "deps/imgui/examples/imgui_impl_sdl.cpp"
ImguiSourceFiles["imgui3"] = "deps/imgui/examples/imgui_impl_opengl3.cpp"
ImguiSourceFiles["imgui4"] = "deps/imgui/imgui.cpp"
ImguiSourceFiles["imgui5"] = "deps/imgui/imgui_draw.cpp"
ImguiSourceFiles["imgui6"] = "deps/imgui/imgui_demo.cpp"


project "FightingGameMain"
    location "."
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
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "include",
        "%{ImguiSourceFiles.imgui1}",
        "%{ImguiSourceFiles.imgui2}",
        "%{ImguiSourceFiles.imgui3}",
        "%{ImguiSourceFiles.imgui4}",
        "%{ImguiSourceFiles.imgui5}",
        "%{ImguiSourceFiles.imgui6}",
    }

    includedirs
    {
        "src",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui2}",
        "%{IncludeDir.ggpo}",
        -- "%{IncludeDir.GameNetworkingSockets}",
        -- "/mingw64/include/freetype2"
    }

    libdirs 
    {
        -- "./dlls/"
    }

    links
    {
        -- "GameNetworkingSockets",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "IMGUI_IMPL_OPENGL_LOADER_GLEW",
        --"IMGUI_IMPL_OPENGL_ES2"
    }

    filter "system:windows"
        systemversion "latest"

        --(win libs) do not statically link
        links 
        { 
            --"gdi32", 
            --"kernel32", 
            --"psapi", 
            "opengl32", 
            --"winmm",
            --"libEGL",
            --"libGLESv2"
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
