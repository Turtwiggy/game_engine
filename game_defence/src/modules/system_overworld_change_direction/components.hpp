#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// change an AI's direction every X seconds

struct ApplyForceInDirectionComponent
{
  glm::vec2 tgt_vel{ 0.0f, 0.0f };
};

} // namespace game2d