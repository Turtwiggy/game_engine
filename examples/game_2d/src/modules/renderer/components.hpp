#pragma once

// game headers
#include "helpers/spritemap.hpp"

// engine headers
#include "engine/opengl/shader.hpp"

// c++ lib headers
#include <vector>

namespace game2d {

struct ColourComponent
{
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

  ColourComponent() = default;
  ColourComponent(float r, float g, float b, float a)
  {
    colour.x = r;
    colour.y = g;
    colour.z = b;
    colour.a = a;
  };
  ColourComponent(const ColourComponent& c)
  {
    colour.x = c.colour.x;
    colour.y = c.colour.y;
    colour.z = c.colour.z;
    colour.a = c.colour.a;
  };
  ColourComponent(const glm::vec4& c)
  {
    colour.x = c.x;
    colour.y = c.y;
    colour.z = c.z;
    colour.a = c.a;
  };
};

struct PositionIntComponent
{
  int x = 0;
  int y = 0;
  float dx = 0.0f; // amount to move in the x and y dirs
  float dy = 0.0f;

  PositionIntComponent() = default;
  PositionIntComponent(int x, int y)
    : x(x)
    , y(y){};
};

struct PositionFloat
{
  float x = 0.0f;
  float y = 0.0f;

  PositionFloat() = default;
  PositionFloat(float x, float y)
    : x(x)
    , y(y){};
};

struct RenderSizeComponent
{
  int w = 0;
  int h = 0;
  float dw = 0.0f; // increase size by float?
  float dh = 0.0f; // increase size by float?

  RenderSizeComponent() = default;
  RenderSizeComponent(int w, int h)
    : w(w)
    , h(h){};
};

struct SpriteComponent
{
  sprite::type sprite = sprite::type::EMPTY;

  SpriteComponent() = default;
  SpriteComponent(sprite::type sprite)
    : sprite(sprite){};
};

// struct CircleComponent
// {
//   float thickness = 1.0f;
//   CircleComponent() = default;
//   CircleComponent(const CircleComponent&) = default;
// };

struct TagComponent
{
  std::string tag;
};

struct ZIndex
{
  int index = 0;
  ZIndex() = default;
  ZIndex(int index)
    : index(index){};
};

// texture constants
constexpr int tex_unit_kenny_nl = 0;
constexpr int tex_unit_main_scene = 1;
constexpr int tex_unit_lighting = 2;

// Attributes only updated by renderer system, read by anything.
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

} // namespace game2d