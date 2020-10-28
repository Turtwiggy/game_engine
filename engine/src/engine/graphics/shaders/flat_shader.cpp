
//header
#include "engine/graphics/shaders/flat_shader.hpp"

#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

void FlatShader::load()
{
    shader_ = ResourceManager::load_shader("assets/shaders/blinn-phong/", {"lit.vert", "lit_directional.frag"}, "FlatShader");
}

Shader FlatShader::get_shader() const
{
    return shader_;
}
    
} //namespace fightingengine