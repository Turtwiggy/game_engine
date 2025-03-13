#pragma once

#include <entt/fwd.hpp>

namespace game2d {

entt::entity
spawn_enemy(entt::registry& r, std::string key, float hp);

void
update_spawner_system(entt::registry& r, const float dt);

} // namespace game2d