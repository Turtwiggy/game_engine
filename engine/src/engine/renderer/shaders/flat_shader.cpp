
//header
#include "engine/renderer/shaders/flat_shader.hpp"

namespace fightingengine {

void FlatShader::load(
    const std::string& path, 
    const std::string& vert_name, 
    const std::string& frag_name )
{
    Shader shader = Shader()
        .attach_shader(path + vert_name, OpenGLShaderTypes::VERTEX)
        .attach_shader(path + frag_name, OpenGLShaderTypes::FRAGMENT)
        .build_program();

    shader_ = shader;
}

Shader FlatShader::get_shader() const
{
    return shader_;
}
    
} //namespace fightingengine