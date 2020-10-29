
//header
#include "engine/scene/background.hpp"

//your project headers
#include "engine/resources/resource_manager.hpp"
#include "engine/mesh/primitives.hpp"
#include "engine/graphics/material.hpp"

namespace fightingengine {

Background::Background()
{
    Shader s = ResourceManager::load_shader(
        "assets/shaders/skybox/", 
        {"skybox.vert", "skybox.frag"}, 
        "skybox-shader");
    shader_ = std::make_shared<Shader>(s);
    
    Material  = fightingengine::Material(shader_);
    Mesh      = new primitives::Cube();
    BoxMin    = glm::vec3(-99999.0);
    BoxMax    = glm::vec3( 99999.0);

    // default material configuration
    Material->set_float("Exposure", 1.0f);
    Material->DepthCompare = GL_LEQUAL;
    Material->Cull = false;
    Material->ShadowCast = false;
    Material->ShadowReceive = false;
}

// void Background::set_cubemap(TextureCube* cubemap)
// {
//     cubemap_ = cubemap;
//     Material->set_texture_cube("background", cubemap, 0);
// }

} //namespace fightingengine