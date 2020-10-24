
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
std::map<std::string, Shader>                  ResourceManager::Shaders;
std::map<std::string, std::shared_ptr<Model>>  ResourceManager::Models;

void ResourceManager::clear()
{
    // delete all shaders	
    for (auto iter : Shaders)
        glDeleteProgram(iter.second.ID);
    // delete all textures
    for (auto iter : Textures)
        glDeleteTextures(1, &iter.second.id);
    // delete all models
    // for (auto iter : Models)
    //     delete iter->second;
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

Texture2D ResourceManager::load_texture(const std::string& full_path, const std::string& unique_name, bool vertically_flip, bool alpha)
{
    Textures[unique_name] = load_texture_from_file(full_path, vertically_flip, alpha);
    printf("texture loaded! %s", unique_name.c_str());
    return Textures[unique_name];
}

Texture2D ResourceManager::get_texture(const std::string& name)
{
    return Textures[name];
}

Texture2D ResourceManager::load_texture_from_file(const std::string& full_path, bool vertically_flip, bool alpha)
{
    printf("----- Texture from path -------\n");
    printf("Dir: %s \n", full_path.c_str());
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
    stbi_set_flip_vertically_on_load(vertically_flip);
    unsigned char* data = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);

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
        printf("FAILED TO LOAD TEXTURE: %s", full_path.c_str());
    }

    // now generate texture
    texture.Generate(width, height, data);

    // and finally free image data
    stbi_image_free(data);

    return texture;
}

// ---- models

std::shared_ptr<Model> ResourceManager::load_model( const std::string& path, const std::string& unique_name )
{
    Models[unique_name] = load_model_from_file(path, unique_name);
    printf("model loaded! %s \n", unique_name.c_str());
    printf("~~~~ end model ~~~~ \n");
    return Models[unique_name];
}

std::shared_ptr<Model> ResourceManager::get_model( const std::string& name )
{
    return Models[name];
}

std::shared_ptr<Model> ResourceManager::load_model_from_file( const std::string& path, const std::string& unique_name )
{
    printf("----- Model from path -------\n");
    printf("Dir: %s \n", path.c_str());

    std::string directory = path.substr(0, path.find_last_of('/'));

    Assimp::Importer import;    
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return nullptr;
    }

    //A model consists of multiple meshes
    std::shared_ptr<Model> model = std::make_shared<Model>();
    model->init(scene, unique_name);    
    return model;
}


} //namespace fightingengine
