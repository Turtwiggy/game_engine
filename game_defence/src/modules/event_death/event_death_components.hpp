#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct DeathEvent
{
  entt::entity dead = entt::null;
};

} // namespace game2d