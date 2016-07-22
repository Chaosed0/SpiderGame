libdir=E:/Libraries/
TARGET=vs2015

while [ "$1" != "" ]; do
    case $1 in
        -d | --libdir )
            shift
            LIBDIR=$1
            ;;
        -t | --target )
            shift
            TARGET=$1
            ;;
        -h | --help )           usage
            exit
            ;;
        * )                     usage
            exit 1
    esac
    shift
done



export FREETYPE_INCLUDEDIR=$libdir/freetype-2.6.3/include/
export ASSIMP_INCLUDEDIR=$libdir/assimp-3.1.1/include/
export BULLET_INCLUDEDIR=$libdir/bullet3/src/
export SDL_INCLUDEDIR=$libdir/SDL2-2.0.4/include/
export SDL_IMAGE_INCLUDEDIR=$libdir/SDL2_image-2.0.1/include/
export GLM_INCLUDEDIR=$libdir/glm/
export GLEW_INCLUDEDIR=$libdir/glew-1.13.0/include/
export NOISE_INCLUDEDIR=$libdir/libnoise/include/

export FREETYPE_LIBDIR_D=$libdir/freetype-2.6.3/objs/vc2010/win32/
export ASSIMP_LIBDIR_D=$libdir/assimp-3.1.1/build/code/Debug/
export BULLET_LIBDIR_D=$libdir/bullet3/lib/Debug/
export SDL_LIBDIR_D=$libdir/SDL2-2.0.4/lib/x86/
export SDL_IMAGE_LIBDIR_D=$libdir/SDL2_image-2.0.1/lib/x86/
export GLEW_LIBDIR_D=$libdir/glew-1.13.0/lib/Release/Win32/
export NOISE_LIBDIR_D=$libdir/libnoise/bin/

export FREETYPE_LIBDIR=$libdir/freetype-2.6.3/objs/vc2010/win32/
export ASSIMP_LIBDIR=$libdir/assimp-3.1.1/build/code/MinSizeRel
export BULLET_LIBDIR=$libdir/bullet3/lib/Release/
export SDL_LIBDIR=$libdir/SDL2-2.0.4/lib/x86/
export SDL_IMAGE_LIBDIR=$libdir/SDL2_image-2.0.1/lib/x86/
export GLEW_LIBDIR=$libdir/glew-1.13.0/lib/Release/Win32/
export NOISE_LIBDIR=$libdir/libnoise/bin/

./premake5.exe $TARGET -f ./premake5.lua
