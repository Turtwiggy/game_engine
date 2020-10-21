
//header
#include "engine/resources/resource_manager.hpp"

//c++ standard library headers
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

//other library headers
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fightingengine {

// Instantiate static variables
std::map<std::string, Texture2D>    ResourceManager::Textures;
std::map<std::string, Shader>       ResourceManager::Shaders;

void ResourceManager::clear()
{
    // delete all shaders	
    for (auto iter : Shaders)
        glDeleteProgram(iter.second.ID);
    // delete all textures
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second.id);
}

// ---- shaders

Shader ResourceManager::load_shader(std::string path, std::vector<std::string> files, std::string name)
{
    Shaders[name] = load_shader_from_file(path, files);
    return Shaders[name];
}

Shader ResourceManager::get_shader(std::string name)
{
    return Shaders[name];
}

Shader ResourceManager::load_shader_from_file(std::string path, std::vector<std::string> files)
{
    printf("----- Shader from path -------\n");
    printf("Dir: %s \n", path.c_str());

    Shader s;

    for (auto& f : files)
    {
        printf("File: %s \n", f.c_str());

        std::string full_path = path + '/' + f;
        OpenGLShaderTypes type = Shader::convert_file_to_shadertype(f);

        s.attach_shader(full_path.c_str(), type);
    }

    printf("-------- Shader ------- \n");

    s.build_program();

    return s;
}

// ---- textures

Texture2D ResourceManager::load_texture(const char* full_path, std::string unique_name, bool flip, bool alpha)
{
    Textures[unique_name] = load_texture_from_file(full_path, flip, alpha);
    printf("texture loaded! %s", unique_name.c_str());
    return Textures[unique_name];
}

Texture2D ResourceManager::get_texture(std::string name)
{
    return Textures[name];
}

Texture2D ResourceManager::load_texture_from_file(const char* full_path, bool flip, bool alpha)
{
    printf("----- Texture from path -------\n");
    printf("Dir: %s \n", full_path);
    printf("----- End Texture -------\n");

    // create texture object
    Texture2D texture(full_path);
    if (alpha)
    {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format = GL_RGBA;
    }
    // load image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char* data = stbi_load(full_path, &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 3)
        {
            texture.Internal_Format = GL_RGB8;
            texture.Image_Format =  GL_RGB;
        }
        else if (nrChannels == 4)
        {
            texture.Internal_Format = GL_RGBA8;
            texture.Image_Format = GL_RGBA;
        }
    }
    else
    {
        printf("FAILED TO LOAD TEXTURE: %s", full_path);
    }

    // now generate texture
    texture.Generate(width, height, data);

    // and finally free image data
    stbi_image_free(data);

    return texture;
}

} //namespace fightingengine
