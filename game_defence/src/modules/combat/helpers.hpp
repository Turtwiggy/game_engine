#pragma once

#include "actors.hpp"
#include "components.hpp"

#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace game2d {

void
create_class(entt::registry& r, entt::entity& e, const EntityType& type, const Weapon& weapon);

std::vector<std::string>
convert_int_to_sprites(int damage);

} // namespace game2d