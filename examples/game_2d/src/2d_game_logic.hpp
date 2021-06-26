#pragma once

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

namespace collisions {
void
resolve(uint32_t id0, uint32_t id1, const std::vector<std::reference_wrapper<GameObject2D>>& ents);
}

namespace enemy_ai {

void
move_along_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s);

void
enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

void
enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

}; // namespace enemy_ai

namespace enemy_spawner {

// spawn a random enemy every X seconds
void
update(std::vector<GameObject2D>& enemies,
       GameObject2D& camera,
       std::vector<GameObject2D>& players,
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

// process input
// ability: boost
// look in mouse direction
// shoot
// update colour
void
update_logic(GameObject2D& player,
             const KeysAndState& keys,
             std::vector<GameObject2D>& bullets,
             const int tex_unit,
             const glm::vec4 col,
             const sprite::type sprite,
             GameObject2D& weapon,
             const float delta_time_s);
}; // namespace player

namespace ui {} // namespace ui

} // namespace game2d