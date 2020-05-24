workspace "FightingGameWorkspace"
    architecture "x86_64"
    startproject "DwarfAndBlade"
    characterset "MBCS"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile",
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["include"] = "deps"
IncludeDir["ImGui"] = "deps/imgui"
IncludeDir["entt"]  = "deps/entt/single_include"
IncludeDir["ggpo"]  = "deps/ggpo/src/include"
IncludeDir["GameNetworkingSockets"] = "deps/gamenetworkingsockets/include"
 
ImguiSourceFiles = {}
ImguiSourceFiles["imgui1"] = "deps/imgui/examples/imgui_impl_glfw.cpp"
ImguiSourceFiles["imgui2"] = "deps/imgui/examples/imgui_impl_opengl3.cpp"
ImguiSourceFiles["imgui3"] = "deps/imgui/misc/freetype/imgui_freetype.cpp" 
ImguiSourceFiles["imgui4"] = "deps/imgui/imgui.cpp"
ImguiSourceFiles["imgui5"] = "deps/imgui/imgui_widgets.cpp"
ImguiSourceFiles["imgui6"] = "deps/imgui/imgui_draw.cpp"
ImguiSourceFiles["imgui7"] = "deps/imgui/imgui-SFML.cpp"

project "FightingGameMain"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

    -- pchheader "spkpch.h"
    -- pchsource "SparkEngine/src/spkpch.cpp"

    filter {"toolset:vs*"}
        buildoptions {"/bigobj" , "/permissive-"}

    filter{}

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "%{ImguiSourceFiles.imgui1}",
        "%{ImguiSourceFiles.imgui2}",
        "%{ImguiSourceFiles.imgui3}",
        "%{ImguiSourceFiles.imgui4}",
        "%{ImguiSourceFiles.imgui5}",
        "%{ImguiSourceFiles.imgui6}",
        "%{ImguiSourceFiles.imgui6}",
        "%{ImguiSourceFiles.imgui7}",
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "./src",
        "%{IncludeDir.include}",	
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.ggpo}",
        "%{IncludeDir.GameNetworkingSockets}",
        -- "/mingw64/include/freetype2"
    }

    libdirs 
    {
        "./deps/gamenetworkingsockets/build/src/"
    }

    links
    {
        "opengl32", --(win lib) do not statically link
        "winmm", --(win lib) do not statically link
    }

    configuration {"gmake2"}
        links
        {
            "mingw32", --do not statically link
            "ssl",
            "glfw3",
            "glew32",
            "sfml-audio",
            "sfml-graphics",
            "sfml-system",
            "sfml-window",
            "harfbuzz",
            "freetype",
            "harfbuzz",
            "freetype",
            "graphite2",
            "flac",
            "png",
            "z",
            "bz2",
            "rpcrt4", --(win lib) do not statically link
            "openal",
            "ogg",
            "ole32", --(win lib) do not statically link
            "dbgeng", --(win lib) do not statically link
            "crypto",
            --"backtrace",
            "gdi32", --(win lib) do not statically link
            "ws2_32",
            "lmdb",
            "libGameNetworkingSockets"
        }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "GLFW_INCLUDE_NONE",
            -- "NO_OPENCL",
            -- "NO_STACKTRACE",
            -- "SUBPIXEL_FONT_RENDERING",
            "IMGUI_IMPL_OPENGL_LOADER_GLEW",
            -- "GLEW_STATIC",
            -- "GRAPHITE2_STATIC",
            -- "SFML_STATIC",
            "__WIN32__",
            -- "ImDrawIdx=unsigned int",
            -- "SERIALISE_ENTT"
        }

        -- postbuildcommands -- copy dll after build
        -- {
        -- 	("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
        -- }

    configuration "Debug"
        defines {"ENGINE_DEBUG", "DEBUG"}
        runtime "Debug"
        symbols "on"

    configuration "gmake2"
        buildoptions 
        {
            "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-O2", "-s"
        }

    configuration "Release"
        defines "ENGINE_RELEASE"
        runtime "Release"
        optimize "on"

        configuration "gmake2"
            buildoptions 
            {
                "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-g", "-Og"
            }