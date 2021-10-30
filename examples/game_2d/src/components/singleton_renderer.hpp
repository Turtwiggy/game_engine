#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// c++ lib headers
#include <vector>

// texture constants
constexpr int tex_unit_kenny_nl = 0;
constexpr int tex_unit_main_scene = 1;
constexpr int tex_unit_lighting = 2;

struct SINGLETON_RendererInfo
{
  // fbo
  unsigned int fbo_main_scene;
  unsigned int fbo_lighting;
  // shaders
  engine::Shader instanced;
  engine::Shader fan;
  // textures
  unsigned int tex_id_main_scene = 0;
  unsigned int tex_id_lighting = 0;
  std::vector<unsigned int> loaded_texture_ids;
  // viewport
  // note: values are updated in render
  glm::vec2 viewport_size_render_at = { 0, 0 };
  glm::vec2 viewport_size_current = { 0, 0 };
  glm::vec2 viewport_pos = { 0, 0 };
  bool viewport_process_events = false;
};