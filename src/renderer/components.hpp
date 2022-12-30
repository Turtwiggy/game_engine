#pragma once

// engine headers
#include "opengl/shader.hpp"

// other
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct TagComponent
{
  std::string tag;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TagComponent, tag);
};

struct TransformComponent
{
  glm::ivec3 position{ 0, 0, 0 };
  glm::vec3 position_dxdy{ 0.0f, 0.0f, 0.0f };
  glm::vec3 rotation_radians = { 0, 0, 0 };
  glm::ivec3 scale{ 0, 0, 0 };
  glm::vec3 scale_dxdy{ 0.0f, 0.0f, 0.0f };

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TransformComponent,
                                 position.x,
                                 position.y,
                                 position.z,
                                 rotation_radians.x,
                                 rotation_radians.y,
                                 rotation_radians.z,
                                 scale.x,
                                 scale.y,
                                 scale.z);
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
struct TextureId
{
  int id = 0;
};

struct Texture
{
  std::string path;
  std::string spritesheet_path;
  TextureUnit tex_unit;
  TextureId tex_id;

  Texture() = default;
  Texture(const std::string& p, const std::string& sp): path(p), spritesheet_path(sp) {};
};

// Attributes only updated by renderer system, read by anything.
struct SINGLETON_RendererInfo
{
  // fbo
  unsigned int fbo_linear_main_scene = 0;
  unsigned int fbo_linear_lighting = 0;
  unsigned int fbo_srgb_main_scene = 0;
  int tex_unit_main = 0;
  int tex_unit_lighting = 0;
  int tex_unit_srgb = 0;
  int tex_id_main = 0;
  int tex_id_lighting = 0;
  int tex_id_srgb = 0;
  // shaders
  engine::Shader instanced;
  engine::Shader fan;
  engine::Shader linear_to_srgb;
  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };

  bool viewport_hovered = false;
  bool viewport_focused = false;
};

} // namespace game2d