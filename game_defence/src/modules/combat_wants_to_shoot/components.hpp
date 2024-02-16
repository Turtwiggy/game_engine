#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

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
  std::optional<glm::ivec2> target = std::nullopt;
};

struct LineOfSightComponent
{
  bool placeholder = true;
};

} // namespace game2d