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
       glm::ivec2 screen_wh,
       float safe_radius_around_player,
       int tex_unit,
       glm::vec4 col,
       sprite::type sprite,
       float delta_time_s);

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
             int tex_unit,
             glm::vec4 col,
             sprite::type sprite,
             float delta_time_s);
}; // namespace player

namespace ui {} // namespace ui

} // namespace game2d