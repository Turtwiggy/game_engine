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
spawn_player(entt::registry& r, std::string key, int num, std::string hull_key, std::string weapon_key, const glm::vec2 pos);

} // namespace game2d