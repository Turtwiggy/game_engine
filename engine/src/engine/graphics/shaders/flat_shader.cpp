
//header
#include "engine/graphics/shaders/flat_shader.hpp"

#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

Shader FlatShader::create_shader()
{
    return ResourceManager::load_shader(
        "assets/shaders/blinn-phong/", 
        {"lit.vert", "lit_directional.frag"}, 
        "FlatShader" );
}
    
} //namespace fightingengine