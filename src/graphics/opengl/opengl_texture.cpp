#include "opengl_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <cassert>

opengl_texture2D::opengl_texture2D(uint32_t width, uint32_t height)
    : width(width), height(height)
{
    internal_format = GL_RGB8;
    data_format = GL_RGBA;

    glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id);
    glTextureStorage2D(renderer_id, 1, internal_format, width, height);

    glTextureParameteri(renderer_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(renderer_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureParameteri(renderer_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(renderer_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

opengl_texture2D::opengl_texture2D(const char* file_name, const std::string& directory)
{
    this->path = directory + '/' + std::string(file_name);

    std::cout << "----- Texture from path -------" << std::endl;
    std::cout << "Filename: " << file_name << std::endl;
    std::cout << "Dir: " << directory << std::endl;
    std::cout << "Path: " << path << std::endl;
    std::cout << "----- End Texture -------" << std::endl;

    int width, height, nrComponents;
    stbi_uc* data = stbi_load(this->path.c_str(), &width, &height, &nrComponents, 0);
    
    this->width = width;
    this->height = height;

    if (data)
    {
        if (nrComponents == 3)
        {
            internal_format = GL_RGB8;
            data_format = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internal_format = GL_RGBA8;
            data_format = GL_RGBA;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &renderer_id);
        glTextureStorage2D(renderer_id, 1, internal_format, width, height);

        glTextureParameteri(renderer_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(renderer_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureParameteri(renderer_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(renderer_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureSubImage2D(renderer_id, 0, 0, 0, width, height, data_format, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << this->path << std::endl;
    }

    stbi_image_free(data);
}

opengl_texture2D::~opengl_texture2D()
{
    glDeleteTextures(1, &renderer_id);
}

void opengl_texture2D::set_data(void* data, uint32_t size)
{
    uint32_t bpp = data_format == GL_RGBA ? 4 : 3;
    assert(size == width * height * bpp); //"Data must be entire texture!");
    glTextureSubImage2D(renderer_id, 0, 0, 0, width, height, data_format, GL_UNSIGNED_BYTE, data);
}

void opengl_texture2D::bind(uint32_t slot) const
{
    glBindTextureUnit(slot, renderer_id);
}

