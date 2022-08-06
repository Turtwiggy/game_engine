// #pragma once

// // game default states

// constexpr int GAME_OVER_WAVE = 10;
// constexpr int ENEMY_ATTACK_THRESHOLD = 4000;
// constexpr bool SPAWN_ENEMIES = true;
// constexpr int EXTRA_ENEMIES_TO_SPAWN_PER_WAVE = 5;
// constexpr int SECONDS_UNTIL_MAX_DIFFICULTY = 100;
// constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_START = 1.0f;
// constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_END = 0.2f;

// constexpr int ENEMY_BASE_HEALTH = 6;
// constexpr int ENEMY_BASE_DAMAGE = 2;
// constexpr int ENEMY_BASE_SPEED = 50;

// constexpr float screenshake_time = 0.1f;
// constexpr float vfx_flash_time = 0.2f;

// glm::ivec2
// player::rotate_b_around_a(const GameObject2D& a, const GameObject2D& b, float radius, float angle)
// {
//   const glm::vec2 center{ 0.0f, 0.0f };
//   const glm::vec2 offset = glm::vec2(center.x + radius * cos(angle), center.y + radius * sin(angle));

//   // Position b in a circle around the a, and render the b in it's center.
//   const glm::vec2 pos = convert_top_left_to_centre(a);
//   const glm::vec2 rot_pos = pos + offset - (glm::vec2(b.physics_size) / 2.0f);

//   return glm::ivec2(int(rot_pos.x), int(rot_pos.y));
// };

// // your header
// #include "player.hpp"

// void
// position_around_player(GameObject2D& player, GameObject2D& weapon, RangedWeaponStats& stats, KeysAndState& keys)
// {
//   weapon.do_render = true;
//   weapon.angle_radians = keys.angle_around_player;

//   const int radius = stats.radius_offset_from_player;
//   const float angle = keys.angle_around_player;
//   glm::vec2 rot_pos = player::rotate_b_around_a(player, weapon, float(radius), angle);
//   weapon.pos = rot_pos;
// };

// // header
// #include "enemy_spawner.hpp"

// // other project headers
// #include <glm/gtx/norm.hpp>
