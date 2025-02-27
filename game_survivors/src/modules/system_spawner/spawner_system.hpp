#pragma once

#include <entt/entt.hpp>

namespace game2d {

entt::entity
spawn_enemy(entt::registry& r, std::string key, float hp);

void
update_spawner_system(entt::registry& r);

} // namespace game2d