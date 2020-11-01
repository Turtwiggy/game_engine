
//header
#include "engine/resources/resource_manager.hpp"

//c++ standard library headers
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

//other library headers
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace fightingengine {

// Instantiate static variables
std::map<std::string, Texture2D>               ResourceManager::Textures;
std::map<std::string, TextureCube>             ResourceManager::TextureCubes;
std::map<std::string, Shader>                  ResourceManager::Shaders;
//std::map<std::string, std::shared_ptr<Model>>  ResourceManager::Models;

void ResourceManager::clear()
{
    // // delete all shaders	
    // for (auto iter : Shaders)
    //     glDeleteProgram(iter.second.ID);
    // // delete all textures
    // for (auto iter : Textures)
    //     glDeleteTextures(1, &iter.second.id);
    // // delete all models
    // // for (auto iter : Models)
    // //     delete iter->second;
    // delete all texturecubes
}

// ---- shaders

Shader ResourceManager::load_shader(const std::string& path, std::vector<std::string> files, const std::string& name)
{
    Shaders[name] = load_shader_from_file(path, files);
    return Shaders[name];
}

Shader ResourceManager::get_shader(const std::string& name)
{
    return Shaders[name];
}

Shader ResourceManager::load_shader_from_file(const std::string& path, std::vector<std::string> files)
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

Texture2D ResourceManager::load_texture( const std::string& full_path, const std::string& unique_name, GLenum target, GLenum format, bool srgb )
{
    Textures[unique_name] = load_texture_from_file(full_path, target, format, srgb);
    printf("texture loaded! %s \n", unique_name.c_str());
    printf("----- End Texture -------\n");
    return Textures[unique_name];
}

Texture2D ResourceManager::get_texture(const std::string& name)
{
    return Textures[name];
}

Texture2D ResourceManager::load_texture_from_file(std::string full_path, GLenum target, GLenum internalFormat, bool srgb )
{
    printf("----- Texture from path -------\n");
    printf("Dir: %s \n", full_path.c_str());

    // create texture object
    Texture2D texture;
    texture.Target = target;
    texture.InternalFormat = internalFormat;
    if(texture.InternalFormat == GL_RGB || texture.InternalFormat == GL_SRGB)
        texture.InternalFormat = srgb ? GL_SRGB : GL_RGB;
    if (texture.InternalFormat == GL_RGBA || texture.InternalFormat == GL_SRGB_ALPHA)
        texture.InternalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;

    // flip textures on their y coordinate while loading
    stbi_set_flip_vertically_on_load(true);

    // load image
    int width, height, nrChannels;
    void *data = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format;
        if(nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        if (target == GL_TEXTURE_2D)
            texture.Generate(width, height, texture.InternalFormat, format, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        printf("FAILED TO LOAD TEXTURE: %s", full_path.c_str());
    }

    // and finally free image data
    stbi_image_free(data);

    return texture;
}

// ---- texture cubes

TextureCube ResourceManager::load_texture_cube(const std::string& folder_path, const std::string& unique_name )
{
    TextureCubes[unique_name] = load_texture_cube_from_folder(folder_path);
    printf("texture cube loaded! %s \n", unique_name.c_str());
    printf("~~~~ Texture Cube from folder ~~~~\n");
    return TextureCubes[unique_name];
}

TextureCube ResourceManager::get_texture_cube(const std::string& name)
{
    return TextureCubes[name];
}

TextureCube ResourceManager::load_texture_cube_from_folder(const std::string& folder)
{
    printf("----- Texture Cube from folder -------\n");

    TextureCube texture;

    // disable y flip on cubemaps
    stbi_set_flip_vertically_on_load(false);

    std::vector<std::string> faces = { "right.jpg", "left.jpg","top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
    for (unsigned int i = 0; i < faces.size(); ++i)
    {
        int width, height, nrComponents;

        std::string path = folder + faces[i];
        printf("Dir: %s \n", path.c_str());

        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        
        if (data)
        {
            GLenum format;
            if (nrComponents == 3)
                format = GL_RGB;
            else
                format = GL_RGBA;

            texture.GenerateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, width, height, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            printf("!! Cube texture at path: %s failed to load. !! \n", faces[i].c_str());
            stbi_image_free(data);
            return texture;
        }
    }
    if(texture.Mipmapping)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return texture;
}

// ---- models

// std::shared_ptr<Model> ResourceManager::load_model( const std::string& path, const std::string& unique_name )
// {
//     Models[unique_name] = load_model_from_file(path, unique_name);
//     printf("model loaded! %s \n", unique_name.c_str());
//     printf("~~~~ end model ~~~~ \n");
//     return Models[unique_name];
// }

// std::shared_ptr<Model> ResourceManager::get_model( const std::string& name )
// {
//     return Models[name];
// }

// std::shared_ptr<Model> ResourceManager::load_model_from_file( const std::string& path, const std::string& unique_name )
// {
//     printf("----- Model from path -------\n");
//     printf("Dir: %s \n", path.c_str());

//     std::string directory = path.substr(0, path.find_last_of('/'));

//     Assimp::Importer import;    
//     const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

//     if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//     {
//         std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
//         return nullptr;
//     }

//     //A model consists of multiple meshes
//     std::shared_ptr<Model> model = std::make_shared<Model>();
//     model->init(scene, unique_name);    
//     return model;
// }


} //namespace fightingengine
