
// header
#include "engine/scene/background.hpp"

// your project headers
#include "engine/graphics/material.hpp"
#include "engine/mesh/primitives.hpp"
#include "engine/resources/resource_manager.hpp"

namespace fightingengine {

// TODO(Turtwiggy) this should not instantiate to an id of 0, it should be
//                 linked to a global counter of scene nodes.
//                 when scenes are a thing... change this!

Background::Background()
  : SceneNode(0)
{

  Shader s = ResourceManager::load_shader(
    "assets/shaders/skybox/", { "skybox.vert", "skybox.frag" }, "skybox-shader");
  shader_ = std::make_shared<Shader>(s);

  Material = std::make_shared<fightingengine::Material>(shader_);
  Mesh = std::make_shared<primitives::Cube>();
  BoxMin = glm::vec3(-99999.0);
  BoxMax = glm::vec3(99999.0);

  // default material configuration
  Material->set_float("Exposure", 1.0f);
  Material->DepthCompare = GL_LEQUAL;
  Material->Cull = false;
  Material->ShadowCast = false;
  Material->ShadowReceive = false;
}

Background::load_background(std::string path, std::string file_name)
{
  // ResourceManager::load_texture_cube("assets/skybox/skybox-default/",
  // "default-skybox"); TextureCube cubemap =
  // ResourceManager::get_texture_cube("default-skybox"); cubemap_ =
  // std::make_shared<TextureCube>(cubemap); background_ = new Background();
  // background_->Material->set_texture_cube("DefaultCubemap", cubemap_.get());
}

// void Background::draw_skybox( const glm::mat4 &view_projection )
// {
//     glm::mat4 model = glm::scale( glm::mat4(1.0f), glm::vec3(1000.0f,
//     1000.0f, 1000.0f) );

//     glDepthFunc( background_->Material->DepthCompare );

//     background_->Material->get_shader()->bind();
//     background_->Material->get_shader()->set_mat4( "view_projection",
//     view_projection ); background_->Material->get_shader()->set_mat4(
//     "model", model );

//     // skybox cube
//     auto *samplers = background_->Material->get_sampler_uniforms();
//     for (auto it = samplers->begin(); it != samplers->end(); ++it)
//     {
//         if ( it->second.Type == SHADER_TYPE::SHADER_TYPE_SAMPLERCUBE )
//             it->second.TextureCube->Bind(it->second.Unit);
//         else
//             it->second.Texture->bind(it->second.Unit);
//     }

//     render_mesh( background_->Mesh );

//     glBindVertexArray(0);
//     glDepthFunc(GL_LESS); // set depth function back to default
// }

// void Background::set_cubemap(TextureCube* cubemap)
// {
//     cubemap_ = cubemap;
//     Material->set_texture_cube("background", cubemap, 0);
// }

} // namespace fightingengine