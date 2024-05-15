#pragma once

#include "glm/glm.hpp"
#include <entt/entt.hpp>

#include <optional>

namespace game2d {

struct FollowTargetComponent
{
  entt::entity target;
};

struct HasTargetPositionComponent
{
  std::optional<glm::ivec2> position = std::nullopt;
};

struct SetVelocityToTargetComponent
{
  bool placeholder = true;
};

} // namespace game2d