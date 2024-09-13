#pragma once

#include "components.hpp"

#include "modules/combat_damage/components.hpp"

#include "entt/entt.hpp"
#include <glm/glm.hpp>

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s, const bool load_saved = false);

// e.g. menu to overworld
void
move_to_scene_additive(entt::registry&, const Scene& s);

entt::entity
add_player_shotgun(entt::registry& r, const entt::entity& e);

} // namespace game2dw