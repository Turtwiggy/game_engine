#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
emplace_or_replace_physics_world(entt::registry& r);

entt::entity
get_fixture_by_tag(entt::registry& r, entt::entity e, std::string tag);

} // namespace game2d