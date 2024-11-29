#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <SDL_scancode.h>

namespace game2d {

struct ActionKey
{
  SDL_Scancode key;
};

struct TransformOffset
{
  glm::vec2 max_offset{ 0, 0 };
  float t = 0.0f;
};

struct BrawlerAI
{
  float time_between_punches_min = 0.4f;
  float time_between_punches_max = 2.0f;
  float time_between_punches_left = 1.0f;
};

struct WantsToPunch
{
  bool placeholder = true;
};

} // namespace game2d