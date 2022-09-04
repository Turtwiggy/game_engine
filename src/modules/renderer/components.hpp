#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// other
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct TagComponent
{
  std::string tag;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TagComponent, tag)
};

struct TransformComponent
{
  glm::ivec3 position{ 0, 0, 0 };
  glm::vec3 position_dxdy{ 0.0f, 0.0f, 0.0f };
  glm::vec3 rotation_radians = { 0, 0, 0 };
  glm::ivec3 scale{ 0, 0, 0 };
  glm::vec3 scale_dxdy{ 0.0f, 0.0f, 0.0f };
};

// Attributes only updated by renderer system, read by anything.
struct SINGLETON_RendererInfo
{
  // fbo
  unsigned int fbo_linear_main_scene = 0;
  unsigned int fbo_linear_lighting = 0;
  unsigned int fbo_srgb_main_scene = 0;
  // shaders
  engine::Shader instanced;
  engine::Shader fan;
  engine::Shader linear_to_srgb;
  // viewport
  // note: values are updated in render
  glm::ivec2 viewport_size_render_at = { 0, 0 };
  glm::ivec2 viewport_size_current = { 0, 0 };
  glm::ivec2 viewport_pos = { 0, 0 };
  bool viewport_process_events = false;
};

} // namespace game2d