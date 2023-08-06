#pragma once

#include "actors.hpp"
#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
create_class(entt::registry& r, entt::entity& e, const EntityType& type, const Weapon& weapon);

} // namespace game2d