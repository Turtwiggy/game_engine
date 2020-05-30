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
ImguiSourceFiles["imgui1"] = "deps/imgui/examples/imgui_impl_sdl.cpp"
ImguiSourceFiles["imgui2"] = "deps/imgui/examples/imgui_impl_opengl3.cpp"
ImguiSourceFiles["imgui3"] = "deps/imgui/misc/freetype/imgui_freetype.cpp" 
ImguiSourceFiles["imgui4"] = "deps/imgui/imgui.cpp"
ImguiSourceFiles["imgui5"] = "deps/imgui/imgui_widgets.cpp"
ImguiSourceFiles["imgui6"] = "deps/imgui/imgui_draw.cpp"
-- ImguiSourceFiles["imgui7"] = "deps/imgui/imgui_demo.cpp"

local BGFX_DIR = "deps/bgfx"
local BIMG_DIR = "deps/bimg"
local BX_DIR = "deps/bx"

function setBxCompat()
    filter "action:vs*"
        includedirs { path.join(BX_DIR, "include/compat/msvc") }
    filter { "system:windows", "action:gmake" }
        includedirs { path.join(BX_DIR, "include/compat/mingw") }
    filter { "system:macosx" }
        includedirs { path.join(BX_DIR, "include/compat/osx") }
        buildoptions { "-x objective-c++" }
end

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
        "%{ImguiSourceFiles.imgui1}",
        "%{ImguiSourceFiles.imgui2}",
        "%{ImguiSourceFiles.imgui3}",
        "%{ImguiSourceFiles.imgui4}",
        "%{ImguiSourceFiles.imgui5}",
        "%{ImguiSourceFiles.imgui6}",
        "%{ImguiSourceFiles.imgui6}",
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS"
    }

    includedirs
    {
        "src",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGui2}",
        "%{IncludeDir.ggpo}",
        "%{IncludeDir.GameNetworkingSockets}",
        path.join(BGFX_DIR, "include"),
		path.join(BX_DIR, "include"),
		path.join(GLFW_DIR, "include")
        -- "/mingw64/include/freetype2"
    }

    libdirs 
    {
        "./dlls/"
    }

    links
    {
        "GameNetworkingSockets",
        "bgfx", 
        "bimg", 
        "bx"
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
            "gdi32", 
            "kernel32", 
            "psapi", 
            "opengl32", 
            "winmm",
        }

        defines
        {
            "__WIN32__"
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

    -- postbuildcommands -- copy dll after build
    -- {
    -- 	("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\"")
    -- }

    -- configuration "gmake2"
    --     buildoptions 
    --     {
    --         "-std=c++17", "-Wall", "-Wextra", "-Wformat", "-O2", "-s"
    --     }
    --     links { }

    filter{}

    setBxCompat()
	
project "bgfx"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
    defines "__STDC_FORMAT_MACROS"
    
    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		path.join(BGFX_DIR, "include/bgfx/**.h"),
		path.join(BGFX_DIR, "src/*.cpp"),
		path.join(BGFX_DIR, "src/*.h"),
	}
	excludes
	{
		path.join(BGFX_DIR, "src/amalgamated.cpp"),
	}
	includedirs
	{
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
		path.join(BGFX_DIR, "include"),
		path.join(BGFX_DIR, "3rdparty"),
		path.join(BGFX_DIR, "3rdparty/dxsdk/include"),
		path.join(BGFX_DIR, "3rdparty/khronos")
    }
    
	filter "configurations:Debug"
        defines "BGFX_CONFIG_DEBUG=1"     
        buildoptions "/MDd"
	filter "action:vs*"
		defines "_CRT_SECURE_NO_WARNINGS"
		excludes
		{
			path.join(BGFX_DIR, "src/glcontext_glx.cpp"),
			path.join(BGFX_DIR, "src/glcontext_egl.cpp")
        }
	filter "system:macosx"
		files
		{
			path.join(BGFX_DIR, "src/*.mm"),
        }

    filter {}

	setBxCompat()

project "bimg"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
    rtti "Off"
    
    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		path.join(BIMG_DIR, "include/bimg/*.h"),
		path.join(BIMG_DIR, "src/image.cpp"),
		path.join(BIMG_DIR, "src/image_gnf.cpp"),
		path.join(BIMG_DIR, "src/*.h"),
		path.join(BIMG_DIR, "3rdparty/astc-codec/src/decoder/*.cc")
	}
	includedirs
	{
		path.join(BX_DIR, "include"),
		path.join(BIMG_DIR, "include"),
		path.join(BIMG_DIR, "3rdparty/astc-codec"),
		path.join(BIMG_DIR, "3rdparty/astc-codec/include"),
    }

    filter "configurations:Debug"
        buildoptions "/MDd"
    
    filter {}

	setBxCompat()

project "bx"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	exceptionhandling "Off"
	rtti "Off"
    defines "__STDC_FORMAT_MACROS"
    
    targetdir ("builds/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("builds/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		path.join(BX_DIR, "include/bx/*.h"),
		path.join(BX_DIR, "include/bx/inline/*.inl"),
		path.join(BX_DIR, "src/*.cpp")
	}
	excludes
	{
		path.join(BX_DIR, "src/amalgamated.cpp"),
		path.join(BX_DIR, "src/crtnone.cpp")
	}
	includedirs
	{
		path.join(BX_DIR, "3rdparty"),
		path.join(BX_DIR, "include")
    }
    
	filter "action:vs*"
        defines "_CRT_SECURE_NO_WARNINGS"
    filter "configurations:Debug"
        buildoptions "/MDd"

    filter {}

	setBxCompat()