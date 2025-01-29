#pragma once

#include "modules/combat_projectiles/projectile_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

entt::entity
spawn_projectile(entt::registry& r, const BulletDef& bullet_def, glm::vec2 pos);

} // namespace game2d