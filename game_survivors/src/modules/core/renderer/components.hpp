#pragma once

#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "modules/core/renderer/fluidsim/components.hpp"
#include "modules/core/renderer/helpers/batch_quad.hpp"
#include "modules/core/renderer/helpers/batch_triangle.hpp"

#include "imgui.h"
#include <entt/fwd.hpp>
#include <glm/fwd.hpp>
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

enum class PassName
{
  water,
  island_triangles,
  island_triangles_gradient,
  island_hidden,
  island_above_hidden,
  island_shore,
  linear_main,
  sprites_to_outline,
  outline,
  sprites_with_shield,
  shine,
  flame,

  // lighting_emitters_and_occluders,
  // voronoi_seed,
  // jump_flood,
  // voronoi_distance,

  mix_lighting_and_scene,
  // crt_effect,

  // blur_pingpong_0,
  // blur_pingpong_1,
  // bloom,
  // fluid_sim,

  count,
};

struct RenderPass
{
  PassName pass;
  std::vector<engine::FramebufferID> fbos;
  bool double_wh = false;

  // one framebuffer can have multiple attachments,
  // in the form of multiple tex_ids.
  std::vector<engine::Texture> texs;

  // the function that gets called during the render
  std::function<void(entt::registry&, float, glm::vec2)> update;

  int colour_buffers_per_texture = 1;
};

void
setup_rp(RenderPass& rp, const glm::ivec2& fbo_size, const int framebuffers = 1);

// Attributes only updated by renderer system, read by anything.
struct SINGLE_RendererInfo
{
  std::vector<RenderPass> passes;

  std::vector<engine::Texture> user_textures;
  int final_jflood_texunit = 0;

  // int tex_unit_circles = 0;
  int tex_unit_ubo_data = 0;

  // quad renderer
  engine::quad_renderer::QuadRenderer renderer;
  engine::tri_renderer::TriangleRenderer tri_renderer;

  // fluidsim
  // FluidSimData fluid_sim;

  // shaders
  engine::Shader water;
  engine::Shader instanced;
  engine::Shader instanced_tri;
  engine::Shader island_tri_gradient;
  engine::Shader island_tri_hidden;
  engine::Shader island_shore;
  engine::Shader shine;
  engine::Shader flame;
  engine::Shader lighting_emitters_and_occluders;
  engine::Shader voronoi_seed; // this shader stores the uv coordinates in the texture
  engine::Shader jump_flood;
  engine::Shader voronoi_distance;
  engine::Shader mix_lighting_and_scene;
  engine::Shader outline;
  // engine::Shader crt;
  // engine::Shader blur;
  // engine::Shader bloom;

  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };

  bool viewport_hovered = false;
  bool viewport_focused = false;

  static SINGLE_RendererInfo instance;
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
  int gridsize = 64;
};

enum class ZLayer : int
{
  BACKGROUND = -5,
  FLOOR,
  ENVIRONMENT,
  BEHIND_PLAYER,
  PLAYER_GUN_BEHIND_PLAYER,
  DEFAULT = 0,
  PLAYER_GUN_ABOVE_PLAYER,
  PROJECTILE,
  VFX,
  FOREGROUND,
};

struct CircleComponent
{
  glm::vec2 shader_pos{ 0.0f, 0.0f };

  // a radius of 1 will be the tilesize set in the shader e.g. 50
  float radius = 1.0f;
};

struct FloorComponent
{
  bool placeholder = true;
};

} // namespace game2d