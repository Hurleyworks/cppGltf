
workspace "Thirdparty"
	architecture "x64"
	location ("builds")
	vectorextensions "AVX2"

if _ACTION == "vs2019" then
   location ("builds/VisualStudio2019")
end

if _ACTION == "vs2022" then
   location ("builds/VisualStudio2022")
end
	configurations 
	{ 
		"Debug", 
        "Release",
    }
	
	filter "configurations:Debug"    defines { "DEBUG" }  symbols  "On"
    filter "configurations:Release"  defines { "NDEBUG" } optimize "On"
    
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- add projects
include "g3log"
include "stb_image"
include "binarytools"

