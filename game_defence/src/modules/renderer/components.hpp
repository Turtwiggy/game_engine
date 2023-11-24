#pragma once

#include "renderer/transform.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/shader.hpp"

// other
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include <vector>

namespace game2d {

struct TagComponent
{
  std::string tag;
};

inline void
to_json(nlohmann::json& j, const TransformComponent& t)
{
  j["transform"]["x"] = t.position.x;
  j["transform"]["y"] = t.position.y;
  j["transform"]["z"] = t.position.z;
  j["transform"]["r"] = t.rotation_radians.z;
  j["transform"]["sx"] = t.scale.x;
  j["transform"]["sy"] = t.scale.y;
};

inline void
from_json(const nlohmann::json& j, TransformComponent& t)
{
  j.at("transform").at("x").get_to(t.position.x);
  j.at("transform").at("y").get_to(t.position.y);
  j.at("transform").at("z").get_to(t.position.z);
  j.at("transform").at("r").get_to(t.rotation_radians.z);
  j.at("transform").at("sx").get_to(t.scale.x);
  j.at("transform").at("sy").get_to(t.scale.y);
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
  std::string spritesheet_path;
  TextureUnit tex_unit;
  TextureId tex_id;

  Texture() = default;
  Texture(const std::string& p, const std::string& sp)
    : path(p)
    , spritesheet_path(sp){};
};

// Attributes only updated by renderer system, read by anything.
struct SINGLETON_RendererInfo
{
  // fbo
  engine::FramebufferID fbo_linear_main;
  engine::FramebufferID fbo_mix_lighting_and_scene;

  int tex_unit_linear_main = 0;
  int tex_unit_mix_lighting_and_scene = 1;
  int RENDERER_TEX_UNIT_COUNT = 2; // should be the number used by renderer

  int tex_id_linear_main = 0;
  int tex_id_mix_lighting_and_scene = 0;

  std::vector<Texture> user_textures;

  // shaders
  engine::Shader instanced;
  engine::Shader mix_lighting_and_scene;

  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };

  bool viewport_hovered = false;
  bool viewport_focused = false;
};

} // namespace game2d