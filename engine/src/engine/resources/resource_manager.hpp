#pragma once

//c++ standard library header
#include <map>
#include <string>
#include <vector>
#include <memory>

//your project headers
#include "engine/graphics/shader.hpp"
#include "engine/graphics/texture.hpp"
#include "engine/graphics/texture_cube.hpp"
#include "engine/scene/scene_node.hpp"

namespace fightingengine {

//TODO(Turtwiggy) make this class use hashed string ID 
//when needed. For the moment, strings are fine.

//TODO(Turtwiggy) make get_functions return null if object isn't found

// A static singleton ResourceManager class that load Textures 
// and Shaders. Each loaded texture and/or shader is stored 
// for future reference by string handles. All functions 
// and resources are static and no public constructor is defined.
class ResourceManager
{
private:
    // resource storage
    static std::map<std::string, Shader>    Shaders;
    static std::map<std::string, Texture2D> Textures;
    static std::map<std::string, TextureCube> TextureCubes;
    static std::map<std::string, SceneNode*> Meshes;

public:
    // de-allocates all loaded resources
    static void clear();

    // loads (and generates) a shader program from file loading vertex, fragment shader's source code. 
    static Shader load_shader(const std::string& path, std::vector<std::string> files, const std::string& name);
    static Shader get_shader(const std::string& name);

    // loads (and generates) a texture from file
    static Texture2D load_texture(const std::string& full_path, const std::string& unique_name, GLenum target = GL_TEXTURE_2D, GLenum format = GL_RGBA, bool srgb = false);
    static Texture2D get_texture(const std::string& name);
    
    // loads a single texture from file
    //e.g PARAM file: assets/textures/Bamboo/BambooWall_1k_albedo.jpg
    static TextureCube load_texture_cube(const std::string& folder_path, const std::string& unique_name );
    static TextureCube get_texture_cube(const std::string& name);    

    // //e.g PARAM full_path: assets/models/Bamboo/BambooWall_1k_albedo.jpg
    // static SceneNode* load_model(const std::string& path, const std::string& unique_name);
    // static SceneNode* get_model(const std::string& name);

private:
    //hidden constructor as static
    ResourceManager() = default;

    //e.g. PARAM path: assets/shaders/raytraced/
    //e.g. PARAM files: ["example.frag", "example.vert", "example.glsl"]
    static Shader load_shader_from_file(const std::string& path, std::vector<std::string> files);
   
    static Texture2D load_texture_from_file(std::string path, GLenum target, GLenum internalFormat, bool srgb = false);

    static TextureCube load_texture_cube_from_folder(const std::string& folder);

    // static SceneNode* load_model_from_file(const std::string& path);
};

} //namespace fightingengine
