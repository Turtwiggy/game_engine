#pragma once

//c++ standard library headers
#include <vector>
#include <string>
#include <iostream>

//other library headers
#include <glm/glm.hpp>

namespace fightingengine {

enum OpenGLShaderTypes
{
    VERTEX,
    FRAGMENT,
    COMPUTE,
    GEOMETRY
};

class Shader
{
public:
    unsigned int ID;

    Shader() = default;

    Shader& attach_shader(const std::string& path, OpenGLShaderTypes shader_type);
    Shader& build_program();

    //gets the type based off the extention
    //e.g. hello.vert returns OpenGLShaderTypes::VERTEX
    //e.g. hello.frag returns OpenGLShaderTypes::FRAGMENT
    //e.g. hello.glsl returns OpenGLShaderTypes::COMPUTE
    static OpenGLShaderTypes convert_file_to_shadertype(std::string file);

private:

    //this is the latest path added via attach_shader()
    //this is mainly used for debugging 
    std::string latest_path;
    
    bool ok_to_build = true;
    std::vector<unsigned int> shaders;

    unsigned int load_shader(const std::string& path, unsigned int gl_shader_type, std::string type);
    void attach_shaders_to_program();

    // utility function for checking shader compilation/linking errors.
    void check_compile_errors(unsigned int shader, std::string type);

public:

    // activate the shader
    void bind();
    void unbind();

    // utility uniform functions

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    [[nodiscard]] int get_uniform_binding_location(const std::string& name) const;

    [[nodiscard]] int get_compute_buffer_binding_location(const std::string& name) const;
    void set_compute_buffer_bind_location(const std::string& name);
};

} //namespace fightingengine
