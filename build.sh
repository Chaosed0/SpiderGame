libdir=D:/Libraries/
openal_sdk_dir="C:/Program Files (x86)/OpenAL 1.1 SDK"
libsndfile_sdk_dir="C:/Program Files (x86)/Mega-Nerd/libsndfile"
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
export OPENAL_INCLUDEDIR=$openal_sdk_dir/include/
export LIBSNDFILE_INCLUDEDIR=$libsndfile_sdk_dir/include/
export GLM_INCLUDEDIR=$libdir/glm/
export GLEW_INCLUDEDIR=$libdir/glew-1.13.0/include/
export NOISE_INCLUDEDIR=$libdir/libnoise/include/

export FREETYPE_LIBDIR_D=$libdir/freetype-2.6.3/objs/vc2010/win32/
export ASSIMP_LIBDIR_D=$libdir/assimp-3.1.1/build/code/Debug/
export BULLET_LIBDIR_D=$libdir/bullet3/lib/Debug/
export SDL_LIBDIR_D=$libdir/SDL2-2.0.4/lib/x86/
export SDL_IMAGE_LIBDIR_D=$libdir/SDL2_image-2.0.1/lib/x86/
export OPENAL_LIBDIR_D=$openal_sdk_dir/libs/Win32/
export LIBSNDFILE_LIBDIR_D=$libsndfile_sdk_dir/lib/
export GLEW_LIBDIR_D=$libdir/glew-1.13.0/lib/Release/Win32/
export NOISE_LIBDIR_D=$libdir/libnoise/Win32/Debug/

export FREETYPE_LIBDIR=$libdir/freetype-2.6.3/objs/vc2010/win32/
export ASSIMP_LIBDIR=$libdir/assimp-3.1.1/build/code/MinSizeRel
export BULLET_LIBDIR=$libdir/bullet3/lib/Release/
export SDL_LIBDIR=$libdir/SDL2-2.0.4/lib/x86/
export SDL_IMAGE_LIBDIR=$libdir/SDL2_image-2.0.1/lib/x86/
export OPENAL_LIBDIR=$openal_sdk_dir/libs/Win32/
export LIBSNDFILE_LIBDIR=$libsndfile_sdk_dir/lib/
export GLEW_LIBDIR=$libdir/glew-1.13.0/lib/Release/Win32/
export NOISE_LIBDIR=$libdir/libnoise/Win32/Release/

./premake5.exe $TARGET -f ./premake5.lua
