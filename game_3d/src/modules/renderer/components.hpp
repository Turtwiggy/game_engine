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

// known before bind
// i.e. chosen by user
struct TextureUnit
{
  int unit = 0;

  TextureUnit() = default;
  TextureUnit(int unit)
    : unit(unit){};
};

// known after bind
// i.e. chosen by opengl
struct TextureId
{
  int id = 0;
};

struct Texture
{
  std::string path;
  // std::string spritesheet_path;
  TextureUnit tex_unit;
  TextureId tex_id;

  Texture() = default;
  Texture(const std::string& p, const std::string& sp)
    : path(p){};
  // , spritesheet_path(sp){};
};

struct SINGLE_RendererComponent
{
  unsigned int cube_vbo = 0;
  unsigned int cube_vao = 0;

  std::vector<glm::vec3> cubes;
  std::vector<glm::vec3> lights;

  //
  int RENDERER_TEX_UNIT_COUNT = 0; // the the number used by renderer

  std::vector<Texture> user_textures;

  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };
};

}