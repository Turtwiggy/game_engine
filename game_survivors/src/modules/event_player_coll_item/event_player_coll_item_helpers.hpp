#pragma once

#include "modules/resolve_collisions/resolve_collisions_helpers.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct CollInfo
{
  std::set<entt::entity> other;
};

void
handle_player_enter_item(entt::registry& r, const OnCollisionEnter& evt);

void
handle_player_exit_item(entt::registry& r, const OnCollisionExit& evt);

} // namespace game2d