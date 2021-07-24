#pragma once

// c++ lib headers
#include <vector>

// other lib headers
#include <glm/glm.hpp>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"

// game headers
#include "2d_game_object.hpp"
#include "spritemap.hpp"

namespace game2d {

namespace bullet {

void
update(GameObject2D& obj, float delta_time_s);
}; // namespace bullet

namespace camera {

void
update(GameObject2D& camera, const KeysAndState& keys, fightingengine::Application& app, float delta_time_s);

}; // namespace camera

namespace enemy_ai {

void
move_along_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s);

void
enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

void
enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

}; // namespace enemy_ai

namespace enemy_spawner {

void
next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left);

// spawn a random enemy every X seconds
void
update(std::vector<GameObject2D>& enemies,
       std::vector<GameObject2D>& players,
       int& enemies_to_spawn,
       const GameObject2D& camera,
       fightingengine::RandomState& rnd,
       const glm::ivec2 screen_wh,
       const float safe_radius_around_player,
       const int tex_unit,
       const glm::vec4 col,
       const sprite::type sprite,
       const float delta_time_s);

}; // namespace enemy_spawner

namespace player {

void
update_input(GameObject2D& obj, KeysAndState& keys, fightingengine::Application& app, GameObject2D& camera);

void
ability_boost(GameObject2D& player, const KeysAndState& keys, const float delta_time_s);

void
ability_slash(fightingengine::Application& app,
              GameObject2D& player_obj,
              const KeysAndState& keys,
              GameObject2D& weapon,
              float delta_time_s,
              std::vector<Attack>& attacks);

void
ability_shoot(fightingengine::Application& app,
              GameObject2D& entity_to_fire_from,
              int& ammo,
              const KeysAndState& keys,
              std::vector<GameObject2D>& bullets,
              const int tex_unit,
              const glm::vec4 bullet_col,
              const sprite::type sprite,
              const float delta_time_s,
              std::vector<Attack>& attacks);

}; // namespace player

}; // namespace game2d