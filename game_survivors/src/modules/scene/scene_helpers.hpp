#pragma once

#include "modules/scene/scene_components.hpp"

#include "entt/entt.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene& s);

// e.g. menu to overworld
void
move_to_scene_additive(entt::registry&, const Scene& s);

struct PlayerSpawnConfig
{
  std::string key = "actor_player";
  std::string name = "steve";
  int player_idx = 0;
  int colour_idx = 0;
  std::string hull_key = "unknown";
  std::string weapon_key = "unknown";
  glm::vec2 pos;
  bool autofire;
};
entt::entity
spawn_player(entt::registry& r, const PlayerSpawnConfig& config);

void
connect_parent_and_weapon(entt::registry& r, entt::entity e, entt::entity wep_e);

} // namespace game2d