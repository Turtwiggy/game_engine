#pragma once

#include "engine/renderer/texture.hpp"
#include "engine/renderer/shader.hpp"

#include <map>
#include <string>
#include <vector>

namespace fightingengine {

    // A static singleton ResourceManager class that hosts several
    // functions to load Textures and Shaders. Each loaded texture
    // and/or shader is also stored for future reference by string
    // handles. All functions and resources are static and no 
    // public constructor is defined.
    class ResourceManager
    {
    public:
        // resource storage
        static std::map<std::string, Shader>    Shaders;
        static std::map<std::string, Texture2D> Textures;
        // loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
        static Shader    load_shader(std::string path, std::vector<std::string> files, std::string name);
        // retrieves a stored sader
        static Shader    get_shader(std::string name);
        // loads (and generates) a texture from file
        static Texture2D load_texture(const char* full_path, bool alpha, std::string name);
        // retrieves a stored texture
        static Texture2D get_texture(std::string name);
        // properly de-allocates all loaded resources
        static void      clear();
    private:
        // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
        ResourceManager() { }

        //e.g. PARAM path: assets/shaders/raytraced/
        //e.g. PARAM files: ["example.frag", "example.vert", "example.glsl"]
        static Shader    load_shader_from_file(std::string path, std::vector<std::string> files);
        // loads a single texture from file
        static Texture2D load_texture_from_file(const char* full_path, bool alpha);
    };

}
