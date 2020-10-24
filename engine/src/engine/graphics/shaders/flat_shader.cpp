
//header
#include "engine/graphics/shaders/flat_shader.hpp"

namespace fightingengine {

void FlatShader::load(
    const std::string& path, 
    const std::string& vert_name, 
    const std::string& frag_name )
{
    printf("----- shader ----- \n");
    printf("shader vert loading...: %s%s \n", path.c_str(), frag_name.c_str());
    printf("shader frag loading...: %s%s \n", path.c_str(), vert_name.c_str());

    Shader shader = Shader()
        .attach_shader(path + vert_name, OpenGLShaderTypes::VERTEX)
        .attach_shader(path + frag_name, OpenGLShaderTypes::FRAGMENT)
        .build_program();

    printf("~~~~ end shader ~~~~ \n");
    shader_ = shader;
}

Shader FlatShader::get_shader() const
{
    return shader_;
}
    
} //namespace fightingengine