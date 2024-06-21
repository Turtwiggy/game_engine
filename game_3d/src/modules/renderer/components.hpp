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

  int tex_unit_diffuse_spaceship = 0;

  // tex_id known after bind
  int tex_id_diffuse_spaceship = 0;

  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };
};

}