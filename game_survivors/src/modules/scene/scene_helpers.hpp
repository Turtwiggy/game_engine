#pragma once

#include "modules/scene/scene_components.hpp"

#include "entt/entt.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s);

// e.g. menu to overworld
void
move_to_scene_additive(entt::registry&, const Scene& s);

entt::entity
spawn_player(entt::registry& r,
             std::string key,
             std::string name,
             int num,
             int colour_idx,
             std::string hull_key,
             std::string weapon_key,
             const glm::vec2 pos);

void
connect_parent_and_weapon(entt::registry& r, entt::entity e, entt::entity wep_e);

} // namespace game2d