#pragma once

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"

// game headers
#include "2d_game_object.hpp"

namespace game2d {

namespace bullet {
static void
update(GameObject2D& obj, float delta_time_s);
}; // namespace bullet

namespace camera {
static void
update(GameObject2D& camera, const KeysAndState& keys, fightingengine::Application& app, float delta_time_s);
}; // namespace camera

namespace enemy_ai {

static void
move_along_vector(GameObject2D& obj, glm::vec2 dir, float delta_time_s);

static void
enemy_directly_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

static void
enemy_arc_angles_to_player(GameObject2D& obj, GameObject2D& player, float delta_time_s);

}; // namespace enemy_ai

namespace enemy_spawner {

// spawn a random enemy every X seconds
static void
update(std::vector<GameObject2D>& enemies,
       GameObject2D& camera,
       std::vector<GameObject2D>& players,
       fightingengine::RandomState& rnd,
       glm::ivec2 screen_wh,
       float delta_time_s);

}; // namespace enemy_spawner

namespace player {

static void
update_input(GameObject2D& obj, KeysAndState& keys, fightingengine::Application& app, GameObject2D& camera);

// process input
// ability: boost
// look in mouse direction
// shoot
// update colour
static void
update_game_logic(GameObject2D& player,
                  const KeysAndState& keys,
                  std::vector<GameObject2D>& bullets,
                  float delta_time_s);
}; // namespace player

namespace ui {} // namespace ui

} // namespace game2d