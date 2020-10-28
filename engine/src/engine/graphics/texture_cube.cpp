
//header
#include "engine/graphics/texture_cube.hpp"

namespace fightingengine {

void TextureCube::DefaultInitialize(
    unsigned int width, 
    unsigned int height, 
    GLenum format, 
    GLenum type, 
    bool mipmap )
{
    glGenTextures(1, &ID);

    FaceWidth = width;
    FaceHeight = height;
    Format = format;
    Type = type;
    Mipmapping = mipmap;

    if (type == GL_HALF_FLOAT && format == GL_RGB)
        InternalFormat = GL_RGB16F;
    else if (type == GL_FLOAT && format == GL_RGB)
        InternalFormat = GL_RGB32F;
    else if (type == GL_HALF_FLOAT && format == GL_RGBA)
        InternalFormat = GL_RGBA16F;
    else if (type == GL_FLOAT && format == GL_RGBA)
        InternalFormat = GL_RGBA32F;

    Bind();

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FilterMin);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, FilterMax);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);

    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, 
            InternalFormat, 
            width, 
            height, 
            0, 
            format, 
            type, 
            nullptr
        );
    }
    if (mipmap)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void TextureCube::GenerateFace(
    GLenum face, 
    unsigned int width, 
    unsigned int height, 
    GLenum format, 
    GLenum type, 
    unsigned char* data )
{
    if(FaceWidth == 0)
        glGenTextures(1, &ID);

    FaceWidth = width;
    FaceHeight = height;
    Format = format;
    Type = type;

    Bind();

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FilterMin);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, FilterMax);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);

    glTexImage2D(face, 0, format, width, height, 0, format, type, data);
}

void TextureCube::Bind(int slot)
{
    if (slot >= 0)
        glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
}

void TextureCube::Unbind()
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

} //namespace fightingengine