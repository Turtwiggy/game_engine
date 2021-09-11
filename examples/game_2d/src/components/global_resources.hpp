#pragma once

// engine headers
#include "engine/maths_core.hpp"
#include "engine/opengl/shader.hpp"

// c++ lib headers
#include <vector>

namespace game2d {

struct Resources
{
  // fbo
  unsigned int fbo_main_scene;
  unsigned int fbo_lighting;
  // shaders
  engine::Shader instanced;
  engine::Shader fan;
  // rnd
  engine::RandomState rnd;
  // textures
  unsigned int tex_id_main_scene;
  unsigned int tex_id_lighting;
  std::vector<unsigned int> loaded_texture_ids;
};

} // namespace game2d