require "os"

workspace "SpiderGame"
    location "build"
    configurations { "Debug", "Release" }
	
project "Engine"
    kind "StaticLib"
	location "build"
    language "C++"
	includedirs { "Engine/src/", "Engine/include/" }
    buildoptions "-std=c++14"
    targetdir "bin/Engine/%{cfg.buildcfg}"

    files { "Engine/src/**.h", "Engine/src/**.cpp", "Engine/include/**.h" }
    includedirs {
                    os.getenv("FREETYPE_INCLUDEDIR"),
                    os.getenv("ASSIMP_INCLUDEDIR"),
                    os.getenv("BULLET_INCLUDEDIR"),
                    os.getenv("SDL_INCLUDEDIR"),
                    os.getenv("SDL_IMAGE_INCLUDEDIR"),
                    os.getenv("OPENAL_INCLUDEDIR"),
                    os.getenv("LIBSNDFILE_INCLUDEDIR"),
                    os.getenv("GLM_INCLUDEDIR"),
                    os.getenv("GLEW_INCLUDEDIR")
                }
        
    filter "configurations:Debug"
        defines { "DEBUG" }
        debugdir "./"
        flags { "Symbols" }

    filter "configurations:Release"
        debugdir "./"
        optimize "On"


project "SpiderGame"
    kind "ConsoleApp"
	location "build"
    language "C++"
	includedirs { "SpiderGame/src/", "Engine/include/" }
    buildoptions "-std=c++14"
    targetdir "bin/Game/%{cfg.buildcfg}"

    files { "SpiderGame/src/**.h", "SpiderGame/src/**.cpp" }
	links { "BulletDynamics", "BulletCollision", "LinearMath", "SDL2", "SDL2main", "SDL2_image", "OpenAL32", "libsndfile-1", "glew32", "OpenGL32", "libnoise", "Engine" }
    includedirs {
                    os.getenv("BULLET_INCLUDEDIR"),
					os.getenv("GLM_INCLUDEDIR"),
                    os.getenv("SDL_INCLUDEDIR"),
                    os.getenv("SDL_IMAGE_INCLUDEDIR"),
                    os.getenv("NOISE_INCLUDEDIR"),
                    os.getenv("EXTRA_INCLUDEDIR")
                }
        
    filter "configurations:Debug"
        defines { "DEBUG" }
        debugdir "./"
        libdirs {
			    os.getenv("FREETYPE_LIBDIR_D"),
				os.getenv("ASSIMP_LIBDIR_D"),
				os.getenv("BULLET_LIBDIR_D"),
				os.getenv("SDL_LIBDIR_D"),
				os.getenv("SDL_IMAGE_LIBDIR_D"),
				os.getenv("OPENAL_LIBDIR_D"),
				os.getenv("LIBSNDFILE_LIBDIR_D"),
				os.getenv("GLEW_LIBDIR_D"),
            os.getenv("NOISE_LIBDIR_D"),
            os.getenv("EXTRA_LIBDIR_D")
        }
		links { "freetype263d", "assimpd" }
        flags { "Symbols" }

    filter "configurations:Release"
        debugdir "./"
        libdirs {
				os.getenv("FREETYPE_LIBDIR"),
				os.getenv("ASSIMP_LIBDIR"),
				os.getenv("BULLET_LIBDIR"),
				os.getenv("SDL_LIBDIR"),
				os.getenv("SDL_IMAGE_LIBDIR"),
				os.getenv("OPENAL_LIBDIR"),
				os.getenv("LIBSNDFILE_LIBDIR"),
				os.getenv("GLEW_LIBDIR"),
            os.getenv("NOISE_LIBDIR"),
            os.getenv("EXTRA_LIBDIR")
        }
		links { "freetype263", "assimp" }
        optimize "On"
		
    filter "files:assets"
        buildaction "Copy"

    filter "files:Shaders"
        buildaction "Copy"
