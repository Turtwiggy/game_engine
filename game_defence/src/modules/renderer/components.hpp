#pragma once

#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/shader.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"

#include "imgui.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <vector>

namespace game2d {

struct ViewportInfo
{
  ImVec2 pos{ 0, 0 };
  ImVec2 size{ 0, 0 };
  bool focused = false;
  bool hovered = false;
};

// known before bind
// i.e. chosen by user
struct TextureUnit
{
  int unit = 0;

  TextureUnit() = default;
  TextureUnit(int unit)
    : unit(unit) {};
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
    , spritesheet_path(sp) {};
};

enum class PassName
{
  stars,
  debris,
  floor_mask,
  linear_main,
  lighting_emitters_and_occluders,
  voronoi_seed,
  jump_flood,
  voronoi_distance,
  mix_lighting_and_scene,
  // blur_pingpong_0,
  // blur_pingpong_1,
  // bloom,
};

struct RenderPass
{
  PassName pass;
  std::vector<engine::FramebufferID> fbos;

  // one framebuffer can have multiple attachments,
  // in the form of multiple tex_ids.
  std::vector<Texture> texs;

  // the function that gets called during the render
  std::function<void(entt::registry&)> update;

private:
  int colour_buffers_per_texture = 0;

public:
  RenderPass(const PassName& pass, const int colour_attachments = 1);

  void setup(const glm::ivec2& fbo_size, const int framebuffers = 1);
};

// Attributes only updated by renderer system, read by anything.
struct SINGLE_RendererInfo
{
  std::vector<RenderPass> passes;

  std::vector<Texture> user_textures;
  int final_jflood_texunit = 0;

  // quad renderer
  engine::quad_renderer::QuadRenderer renderer;

  // shaders
  engine::Shader stars;
  engine::Shader debris;
  engine::Shader instanced;
  engine::Shader lighting_emitters_and_occluders;
  engine::Shader voronoi_seed; // this shader stores the uv coordinates in the texture
  engine::Shader jump_flood;
  engine::Shader voronoi_distance;
  engine::Shader mix_lighting_and_scene;
  // engine::Shader blur;
  // engine::Shader bloom;

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

struct Effect_BlurInfo
{
  int last_blur_texunit = 0;
};

struct Effect_GridComponent
{
  int gridsize = 50;
};

enum class ZLayer : int
{
  BACKGROUND = -4,
  FLOOR,
  BEHIND_PLAYER,
  PLAYER_GUN_BEHIND_PLAYER,
  DEFAULT = 0,
  PLAYER_HELMET,
  PLAYER_GUN_ABOVE_PLAYER,
  FOREGROUND,
};

struct CircleComponent
{
  glm::vec2 shader_pos{ 0, 0 };

  // a radius of 1 will be the tilesize set in the shader e.g. 50
  float radius = 1.0f;
};

struct FloorComponent
{
  bool placeholder = true;
};

} // namespace game2d