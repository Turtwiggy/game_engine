
//header
#include "engine/graphics/background.hpp"

//your project headers
#include "engine/resources/resource_manager.hpp"
#include "engine/mesh/primitives.hpp"

namespace fightingengine {

Background::Background()
{

    shader_ = ResourcesManager::load_shader(
        "assets/shaders/skybox/", 
        {"skybox.vert", "skybox.frag"}, 
        "skybox-shader");
    
    material_ = Material(shader_);
    mesh_ = primitive::Cube();
    BoxMin    = math::vec3(-99999.0);
    BoxMax    = math::vec3( 99999.0);

    // // default material configuration
    // Material->SetFloat("Exposure", 1.0f);
    // Material->DepthCompare = GL_LEQUAL;
    // Material->Cull = false;
    // Material->ShadowCast = false;
    // Material->ShadowReceive = false;
}

void Background::set_cubemap(TextureCube* cubemap)
{
    cubemap_ = cubemap;
    //Material->SetTextureCube("background", cubemap, 0);
}

} //namespace fightingengine