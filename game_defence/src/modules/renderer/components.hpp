#pragma once

#include "opengl/texture.hpp"
#include "renderer/transform.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/shader.hpp"

// other
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <vector>

namespace game2d {

struct ViewportInfo
{
  ImVec2 pos;
  ImVec2 size;
  bool focused = false;
  bool hovered = false;
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

enum class PassName
{
  stars,
  linear_main,
  lighting_emitters_and_occluders,
  voronoi_seed,
  jump_flood,
  voronoi_distance,
  lighting_ambient_occlusion,
  mix_lighting_and_scene, // 2 colour attachments
  blur_pingpong_0,
  blur_pingpong_1,
  bloom,
};

struct RenderPass
{
  PassName pass;
  std::vector<engine::FramebufferID> fbos;

  // one framebuffer can have multiple attachments,
  // in the form of multiple multiple tex_ids.
  std::vector<Texture> texs;

  // the function that gets called during the render
  std::function<void()> update;

private:
  int colour_buffers_per_texture = 0;

public:
  RenderPass(const PassName& pass, const int colour_attachments = 1);

  void setup(const glm::ivec2& fbo_size, const int framebuffers = 1);
};

// Attributes only updated by renderer system, read by anything.
struct SINGLETON_RendererInfo
{
  std::vector<RenderPass> passes;

  std::vector<Texture> user_textures;

  // shaders
  engine::Shader stars;
  engine::Shader instanced;
  engine::Shader lighting_emitters_and_occluders;
  engine::Shader voronoi_seed; // this shader stores the uv coordinates in the texture
  engine::Shader jump_flood;
  engine::Shader voronoi_distance;
  engine::Shader lighting_ambient_occlusion;
  engine::Shader mix_lighting_and_scene;
  engine::Shader circle;
  engine::Shader grid;
  engine::Shader blur;
  engine::Shader bloom;

  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };

  bool viewport_hovered = false;
  bool viewport_focused = false;
};

struct Effect_DoBloom
{
  bool placeholder = true;
};

struct EffectBlurInfo
{
  int last_blur_texunit = 0;
};

} // namespace game2d