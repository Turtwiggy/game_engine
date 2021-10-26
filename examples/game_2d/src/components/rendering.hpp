#pragma once

// game headers
#include "helpers/spritemap.hpp"

// engine headers
#include "engine/opengl/shader.hpp"

// c++ lib headers
#include <vector>

namespace game2d {

struct Colour
{
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

  Colour() = default;
  Colour(float r, float g, float b, float a)
  {
    colour.x = r;
    colour.y = g;
    colour.z = b;
    colour.a = a;
  };
  Colour(const Colour& c)
  {
    colour.x = c.colour.x;
    colour.y = c.colour.y;
    colour.z = c.colour.z;
    colour.a = c.colour.a;
  };
  Colour(const glm::vec4& c)
  {
    colour.x = c.x;
    colour.y = c.y;
    colour.z = c.z;
    colour.a = c.a;
  };
};

struct PositionInt
{
  int x = 0;
  int y = 0;
  float dx = 0.0f; // remainders in the x and y dirs
  float dy = 0.0f;

  PositionInt() = default;
  PositionInt(int x, int y)
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

struct Size
{
  int w = 0;
  int h = 0;

  Size() = default;
  Size(int w, int h)
    : w(w)
    , h(h){};
};

struct Sprite
{
  sprite::type sprite = sprite::type::EMPTY;

  Sprite() = default;
  Sprite(sprite::type sprite)
    : sprite(sprite){};
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
  glm::vec2 viewport_size = { 0, 0 };
  glm::vec2 viewport_pos = { 0, 0 };
};

} // namespace game2d