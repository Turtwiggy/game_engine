#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

struct AbleToShoot
{
  bool placeholder = true;
};

struct WantsToShoot
{
  bool placeholder = true;
};

struct WantsToReleaseShot
{
  bool placehoolder = true;
};

// a moving target
struct DynamicTargetComponent
{
  entt::entity target = entt::null;
};

struct StaticTargetComponent
{
  std::optional<glm::vec2> target = std::nullopt;
};

} // namespace game2d