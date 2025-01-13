#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

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