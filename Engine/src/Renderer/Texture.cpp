
#include "Texture.h"

TextureImpl::TextureImpl()
	: type(TextureType_diffuse), id(0)
{ }

TextureImpl::TextureImpl(TextureType type, GLuint id)
	: type(type), id(id)
{ }
