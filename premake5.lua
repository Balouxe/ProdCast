workspace "ProdCast"
    architecture "x64"
    configurations {
        "Debug", 
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "ProdCast/vendor/imgui"
include "ProdCast/vendor/glfw"

project "ProdCast"
    location "ProdCast"
    kind "ConsoleApp"
    language "C++"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/vendor/imgui",
        "%{prj.name}/vendor/glfw/include",
        "%{prj.name}/vendor/portaudio"
    }

    links{
        "ImGui",
        "GLFW",
        "opengl32.lib",
        "portaudio_x64.lib"
    }

    filter "system:windows"
        cppdialect "C++20"
        systemversion "latest"
        defines{
            "PC_PLATFORM_WINDOWS"
        }

   filter "configurations:Debug"
      defines { "PC_DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "PC_RELEASE" }
      optimize "On"
      
    filter "configurations:Dist"
      defines { "PC_DIST" }
      optimize "On"