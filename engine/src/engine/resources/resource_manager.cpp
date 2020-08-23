#include "engine/resources/resource_manager.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fightingengine {

    // Instantiate static variables
    std::map<std::string, Texture2D>    ResourceManager::Textures;
    std::map<std::string, Shader>       ResourceManager::Shaders;


    Shader ResourceManager::load_shader(std::string path, std::vector<std::string> files, std::string name)
    {
        Shaders[name] = load_shader_from_file(path, files);
        return Shaders[name];
    }

    Shader ResourceManager::get_shader(std::string name)
    {
        return Shaders[name];
    }

    Texture2D ResourceManager::load_texture(const char* file, bool alpha, std::string name)
    {
        Textures[name] = load_texture_from_file(file, alpha);
        printf("texture loaded! %s", name.c_str());
        return Textures[name];
    }

    Texture2D ResourceManager::get_texture(std::string name)
    {
        return Textures[name];
    }

    void ResourceManager::clear()
    {
        // (properly) delete all shaders	
        for (auto iter : Shaders)
            glDeleteProgram(iter.second.ID);
        // (properly) delete all textures
        for (auto iter : Textures)
            glDeleteTextures(1, &iter.second.id);
    }


    Shader ResourceManager::load_shader_from_file(std::string path, std::vector<std::string> files)
    {
        printf("----- Shader from path -------\n");
        printf("Dir: %s \n", path);

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

    //e.g PARAM file: assets/textures/Bamboo/BambooWall_1k_albedo.jpg
    Texture2D ResourceManager::load_texture_from_file(const char* full_path, bool alpha)
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
}
