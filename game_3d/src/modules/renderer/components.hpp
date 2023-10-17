#pragma once

#include "opengl/shader.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace game2d {

struct TransformComponent
{
  glm::vec3 position{ 0, 0, 0 };
  glm::vec3 rotation = { 0, 0, 0 }; // radians
  glm::vec3 scale{ 0, 0, 0 };
};

struct SINGLE_ShadersComponent
{
  engine::Shader solid_colour{ "assets/shaders/basic.vert", "assets/shaders/solid_colour.frag" };
  engine::Shader basic{ "assets/shaders/basic.vert", "assets/shaders/basic.frag" };
  engine::Shader animated{ "assets/shaders/animated.vert", "assets/shaders/animated.frag" };
};

struct SINGLE_RendererComponent
{
  unsigned int cube_vbo = 0;
  unsigned int cube_vao = 0;

  std::vector<glm::vec3> cubes;
  std::vector<glm::vec3> lights;
};

}