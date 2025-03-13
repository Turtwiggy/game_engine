#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct RequestHitScaleComponent
{
  float t = 0.0f;
};

struct DefaultSizeComponent
{
  glm::vec2 size{ 0, 0 };
};

} // namespace game2d