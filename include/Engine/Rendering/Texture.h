#ifndef Texture_h__
#define Texture_h__

#include <boost/filesystem.hpp>
#include "../OpenGL.h"
#include "BaseTexture.h"
#include "PNG.h"
#include "DDS.h"

class Texture : public BaseTexture
{
	friend class ResourceManager;

protected:
	Texture(std::string path);

public:
	~Texture();

	void Bind(GLenum textureUnit = GL_TEXTURE0);

	GLuint m_Texture = 0;

	enum TextureType : GLint { Invalid = 0, DDS = 1, PNG = 2};
	TextureType m_Type;
};

#endif
