#pragma once

#include "opengl/shader.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace game2d {

struct TransformComponent
{
  glm::vec3 position{ 0, 0, 0 };
  glm::vec3 rotation_radians = { 0, 0, 0 };
  glm::vec3 scale{ 0, 0, 0 };
};

struct SINGLE_ShadersComponent
{
  engine::Shader basic{ "assets/shaders/basic.vert", "assets/shaders/basic.frag" };
};

struct SINGLE_RendererComponent
{
  unsigned int cube_vbo = 0;
  unsigned int cube_vao = 0;

  std::vector<glm::vec3> cubes;
};

}