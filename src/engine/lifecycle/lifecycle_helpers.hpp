#pragma once

#include <entt/fwd.hpp>

namespace game2d {

bool
is_parent_valid(entt::registry& r, entt::entity e);

void
on_parent_destroyed(entt::registry& r, const entt::entity e);

void
on_child_destroyed(entt::registry& r, const entt::entity e);

} // namespace game2d