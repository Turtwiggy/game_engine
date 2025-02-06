#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct DeathEvent
{
  entt::entity killed_by = entt::null;
  entt::entity dead = entt::null;
};

} // namespace game2d