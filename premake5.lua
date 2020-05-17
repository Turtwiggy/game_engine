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
IncludeDir["include"] = "include"
-- IncludeDir["ImGui"] = "include/imgui"
IncludeDir["entt"]  = "include/entt/single_include"

-- Any source files we want included in the project
-- SourceFiles = {}
-- SourceFiles["ndb"]  = "include/ndb/db_storage.cpp"
-- ToolkitSourceFiles = {}
-- ToolkitSourceFiles["toolkit1"] = "include/toolkit/texture.cpp"
-- ToolkitSourceFiles["toolkit2"] = "include/toolkit/base_serialisables.cpp"
-- ToolkitSourceFiles["toolkit3"] = "include/toolkit/clipboard.cpp"
-- ToolkitSourceFiles["toolkit4"] = "include/toolkit/clock.cpp"
-- ToolkitSourceFiles["toolkit5"] = "include/toolkit/fs_helpers.cpp"
-- ToolkitSourceFiles["toolkit6"] = "include/toolkit/opencl.cpp"
-- ToolkitSourceFiles["toolkit7"] = "include/toolkit/render_window.cpp"
-- ToolkitSourceFiles["toolkit8"] = "include/toolkit/stacktrace.cpp"

-- ImguiSourceFiles = {}
-- ImguiSourceFiles["imgui1"] = "include/imgui/examples/imgui_impl_glfw.cpp"
-- ImguiSourceFiles["imgui2"] = "include/imgui/examples/imgui_impl_opengl3.cpp"
-- ImguiSourceFiles["imgui3"] = "include/imgui/misc/freetype/imgui_freetype.cpp" 
-- ImguiSourceFiles["imgui4"] = "include/imgui/imgui.cpp"
-- ImguiSourceFiles["imgui5"] = "include/imgui/imgui_widgets.cpp"
-- ImguiSourceFiles["imgui6"] = "include/imgui/imgui_draw.cpp"

-- NetworkingSourceFiles = {}
-- NetworkingSourceFiles["networking1"] = "include/networking/beast_compilation_unit.cpp"
-- NetworkingSourceFiles["networking2"] = "include/networking/networking.cpp"
-- NetworkingSourceFiles["networking3"] = "include/networking/serialisable.cpp"

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
        -- "%{SourceFiles.toolkit}",
        -- "%{SourceFiles.networking}",
        -- "%{SourceFiles.ndb}",
        -- "%{ImguiSourceFiles.imgui1}",
        -- "%{ImguiSourceFiles.imgui2}",
        -- "%{ImguiSourceFiles.imgui3}",
        -- "%{ImguiSourceFiles.imgui4}",
        -- "%{ImguiSourceFiles.imgui5}",
        -- "%{ImguiSourceFiles.imgui6}",
        -- "%{ImguiSourceFiles.imgui6}",
        -- "%{ToolkitSourceFiles.toolkit1}",
        -- "%{ToolkitSourceFiles.toolkit2}",
        -- "%{ToolkitSourceFiles.toolkit3}",
        -- "%{ToolkitSourceFiles.toolkit4}",
        -- "%{ToolkitSourceFiles.toolkit5}",
        -- "%{ToolkitSourceFiles.toolkit6}",
        -- "%{ToolkitSourceFiles.toolkit7}",
        -- "%{ToolkitSourceFiles.toolkit8}",
        -- "%{NetworkingSourceFiles.networking1}",
        -- "%{NetworkingSourceFiles.networking2}",
        -- "%{NetworkingSourceFiles.networking3}",
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "./src",
        "%{IncludeDir.include}",	
        -- "%{IncludeDir.ImGui}",
        "%{IncludeDir.entt}",
        -- "/mingw64/include/freetype2"
    }

    links
    {
        "opengl32",
    }

    configuration {"gmake2"}
        links
        {
            -- "mingw32",
            -- "ssl",
            -- "glfw3",
            -- "glew32",
            -- "sfml-audio",
            -- "sfml-graphics",
            -- "sfml-system",
            -- "sfml-window",
            -- "harfbuzz",
            -- "freetype",
            -- "harfbuzz",
            -- "freetype",
            -- "graphite2",
            -- "opengl32",
            -- "flac",
            -- "png",
            -- "z",
            -- "bz2",
            -- "rpcrt4",
            -- "openal",
            -- "ogg",
            -- "ole32",
            -- "dbgeng",
            -- "crypto",
            --"backtrace",
            -- "gdi32",
            -- "ws2_32",
            -- "lmdb",
            -- "winmm"
        }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            -- "GLFW_INCLUDE_NONE",
            -- "NO_OPENCL",
            -- "NO_STACKTRACE",
            -- "SUBPIXEL_FONT_RENDERING",
            -- "IMGUI_IMPL_OPENGL_LOADER_GLEW",
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