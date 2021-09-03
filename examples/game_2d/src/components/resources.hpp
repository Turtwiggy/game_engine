#pragma once

#include "engine/opengl/shader.hpp"

namespace game2d {

struct Resources
{
  unsigned int fbo_main_scene;
  unsigned int fbo_lighting;
  unsigned int tex_id_main_scene;
  unsigned int tex_id_lighting;
  std::vector<unsigned int> texture_ids;

  fightingengine::Shader colour_shader;
  fightingengine::Shader fan_shader;
  fightingengine::Shader instanced_quad_shader;

  Resources(fightingengine::Shader c, fightingengine::Shader f, fightingengine::Shader i)
    : colour_shader(c)
    , fan_shader(f)
    , instanced_quad_shader(i){};
};

} // namespace game2d