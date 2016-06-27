require "os"
require "string"

solution "OpenGLTest"
	configurations { "Debug", "Release" }
	location "build"
	includedirs "src"
	
	project "OpenGLTest"
		language "C++"
		files "src/**.cpp"
		links { "BulletDynamics", "BulletCollision", "LinearMath", "SDL2", "SDL2main", "SDL2_image", "glew32", "OpenGL32", "libnoise" }
		includedirs {
						os.getenv("FREETYPE_INCLUDEDIR"),
						os.getenv("ASSIMP_INCLUDEDIR"),
						os.getenv("BULLET_INCLUDEDIR"),
						os.getenv("SDL_INCLUDEDIR"),
						os.getenv("SDL_IMAGE_INCLUDEDIR"),
						os.getenv("GLM_INCLUDEDIR"),
						os.getenv("GLEW_INCLUDEDIR"),
						os.getenv("NOISE_INCLUDEDIR"),
						os.getenv("EXTRA_INCLUDEDIR")
					}
		kind "WindowedApp"
		buildoptions "-std=c++11"
			
		configuration "Debug"
            targetdir "bin/debug"
			libdirs {
				os.getenv("FREETYPE_LIBDIR_D"),
				os.getenv("ASSIMP_LIBDIR_D"),
				os.getenv("BULLET_LIBDIR_D"),
				os.getenv("SDL_LIBDIR_D"),
				os.getenv("SDL_IMAGE_LIBDIR_D"),
				os.getenv("GLEW_LIBDIR_D"),
				os.getenv("NOISE_LIBDIR_D"),
				os.getenv("EXTRA_LIBDIR_D")
			}
			links { "freetype263d", "assimpd" }
			flags { "Symbols", "ExtraWarnings" }

		configuration "Release"
            targetdir "bin/release"
			libdirs {
				os.getenv("FREETYPE_LIBDIR"),
				os.getenv("ASSIMP_LIBDIR"),
				os.getenv("BULLET_LIBDIR"),
				os.getenv("SDL_LIBDIR"),
				os.getenv("SDL_IMAGE_LIBDIR"),
				os.getenv("GLEW_LIBDIR"),
				os.getenv("NOISE_LIBDIR"),
				os.getenv("EXTRA_LIBDIR")
			}
			links { "freetype263", "assimp" }
			flags { "Optimize", "ExtraWarnings" }
