#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct RequestHitScaleComponent
{
  float t = 0.0f;
  float scale_up_pixels = 4.0f; // e.g. if you're 16px, scale up to 18px
};

struct DefaultSizeComponent
{
  glm::vec2 size{ 0, 0 };
};

} // namespace game2d