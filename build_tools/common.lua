
local ROOT = "../"

	language "C++"

	defines{
	 "_USE_MATH_DEFINES"
	}
	flags { "MultiProcessorCompile", "NoMinimalRebuild" }
	
	local CORE_DIR = ROOT .. "core/source/"
	local JAHLEY_DIR = ROOT .. "core/source/jahley/"
	local THIRD_PARTY_DIR = "../thirdparty/"
	local MODULE_DIR = "../modules/"
	
	includedirs
	{
		CORE_DIR,
		JAHLEY_DIR,
		MODULE_DIR,
	
		THIRD_PARTY_DIR,
		THIRD_PARTY_DIR .. "g3log/src",
		THIRD_PARTY_DIR .. "json",
		THIRD_PARTY_DIR .. "stb_image",
		THIRD_PARTY_DIR .. "binarytools/src",
	}
	
	targetdir (ROOT .. "builds/bin/" .. outputdir .. "/%{prj.name}")
	objdir (ROOT .. "builds/bin-int/" .. outputdir .. "/%{prj.name}")
	
	filter { "system:windows"}
		disablewarnings { 
			"5030", "4244", "4267", "4667", "4018", "4101", "4305", "4316", "4146", "4996", "4554",
		} 
		linkoptions { "-IGNORE:4099" } -- can't find debug file in release folder
		characterset ("MBCS")
		buildoptions { "/Zm250", "/bigobj", "/Zc:__cplusplus"}
		
		defines 
		{ 
			"WIN32", "_WINDOWS",
			--https://github.com/KjellKod/g3log/issues/337
			"_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING",
			"CHANGE_G3LOG_DEBUG_TO_DBUG",
		}
		
	filter "configurations:Debug"
	
		postbuildcommands {
			
		}
		links 
		{ 
			"Core",
			"g3log",
			"stb_image",
			"binarytools",
		}
		defines { "DEBUG", "USE_DEBUG_EXCEPTIONS" }
		symbols "On"
		libdirs { THIRD_PARTY_DIR .. "builds/bin/" .. outputdir .. "/**",
				  ROOT .. "builds/bin/" .. outputdir .. "/**"
				 
		}
		
	filter "configurations:Release"
	postbuildcommands {
			
		}
		links 
		{ 
			"Core",
			"g3log",
			"stb_image",
			"binarytools",
		}
		defines { "NDEBUG"}
		optimize "On"
		libdirs { THIRD_PARTY_DIR .. "builds/bin/" .. outputdir .. "/**",
				  ROOT .. "builds/bin/" .. outputdir .. "/**"
		}
	
	  


	 		

