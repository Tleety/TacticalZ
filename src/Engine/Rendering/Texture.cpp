#include "Rendering/Texture.h"

Texture::Texture(std::string path)
{
    PNG* img = ResourceManager::Load<PNG, true>(path); //TODO: Make this threaded. Catch exeptions in all other load places.

    this->Width = img->Width;
    this->Height = img->Height;
    this->Data = img->Data;

    GLint format;
    switch (img->Format) {
    case Image::ImageFormat::RGB:
        format = GL_RGB;
        break;
    case Image::ImageFormat::RGBA:
        format = GL_RGBA;
        break;
    }
   
    // Construct the OpenGL texture
    glGenTextures(1, &m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, img->Width, img->Height, 0, format, GL_UNSIGNED_BYTE, img->Data);
	glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLERROR("Texture load");

    ResourceManager::Release("PNG", path);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_Texture);
}

void Texture::Bind(GLenum textureUnit /* = GL_TEXTURE0 */)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
}
