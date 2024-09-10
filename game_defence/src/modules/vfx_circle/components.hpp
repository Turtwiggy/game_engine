#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct CircleComponent
{
  glm::vec2 shader_pos{ 0, 0 };

  // a radius of 1 will be the tilesize set in the shader e.g. 50
  float radius = 1.0f;
};

};