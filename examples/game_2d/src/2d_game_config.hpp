#pragma once

#include "2d_game_object.hpp"
#include "spritemap.hpp"
#include <map>

// game default states

const int GAME_GRID_SIZE = 32;
const int GAME_OVER_WAVE = 10;
const float ENEMY_ATTACK_THRESHOLD = 4000.0f;

const bool SPAWN_ENEMIES = true;
const int EXTRA_ENEMIES_TO_SPAWN_PER_WAVE = 5;
const float SECONDS_UNTIL_MAX_DIFFICULTY = 100.0f;
const float SECONDS_BETWEEN_SPAWNING_ENEMIES_START = 1.0f;
const float SECONDS_BETWEEN_SPAWNING_ENEMIES_END = 0.2f;

// vfx
const float screenshake_time = 0.1f;
const float vfx_flash_time = 0.2f;

// shop & stats
const int shop_refill_pistol_ammo = 5;
const int shop_refill_shotgun_ammo = 5;
const int shop_refill_machinegun_ammo = 5;

// textures
const int tex_unit_kenny_nl = 0;
const int tex_unit_main_scene = 1;
const int tex_unit_lighting = 2;

// sprites
const game2d::sprite::type sprite_player = game2d::sprite::type::SQUARE;
const game2d::sprite::type sprite_tree = game2d::sprite::type::SQUARE;
const game2d::sprite::type sprite_weapon_base = game2d::sprite::type::WEAPON_DAGGER_SOLID_3;
const game2d::sprite::type sprite_pistol = game2d::sprite::type::WEAPON_PISTOL;
const game2d::sprite::type sprite_shotgun = game2d::sprite::type::WEAPON_SHOTGUN;
const game2d::sprite::type sprite_machinegun = game2d::sprite::type::WEAPON_SUB_MACHINE_GUN;
const game2d::sprite::type sprite_bullet = game2d::sprite::type::SQUARE;
const game2d::sprite::type sprite_enemy_core = game2d::sprite::type::SQUARE;
const game2d::sprite::type sprite_splat = game2d::sprite::type::SQUARE;
const game2d::sprite::type sprite_heart_1 = game2d::sprite::type::ICON_HEART;
const game2d::sprite::type sprite_heart_2 = game2d::sprite::type::ICON_HEART_OUTLINE;
const game2d::sprite::type sprite_heart_3 = game2d::sprite::type::ICON_HEART_HALF_FULL;
const game2d::sprite::type sprite_heart_4 = game2d::sprite::type::ICON_HEART_FULL;

// colour palette; https://colorhunt.co/palette/273312
const glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);    // black
const glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f);   // blue
const glm::vec4 PALETTE_COLOUR_3_1 = glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
const glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey
// colours: entities
const glm::vec4 background_colour = PALETTE_COLOUR_1_1; // black
const glm::vec4 debug_line_colour = PALETTE_COLOUR_2_1; // blue
const glm::vec4 player_colour = PALETTE_COLOUR_2_1;     // blue
const glm::vec4 enemy_colour = PALETTE_COLOUR_4_1;      // grey
// colours: weapons
const glm::vec4 weapon_shovel_colour = PALETTE_COLOUR_3_1;
const glm::vec4 weapon_shovel_flash_colour = PALETTE_COLOUR_2_1;
const glm::vec4 weapon_pistol_colour = PALETTE_COLOUR_3_1;
const glm::vec4 weapon_pistol_flash_colour = PALETTE_COLOUR_2_1;
const glm::vec4 weapon_shotgun_colour = glm::vec4(1.0, 1.0, 0.0, 1.0);
const glm::vec4 weapon_shotgun_flash_colour = PALETTE_COLOUR_2_1;
const glm::vec4 weapon_machinegun_colour = PALETTE_COLOUR_3_1;
const glm::vec4 weapon_machinegun_flash_colour = PALETTE_COLOUR_2_1;
const glm::vec4 bullet_pistol_colour = weapon_pistol_colour;
const glm::vec4 bullet_shotgun_colour = weapon_shotgun_colour;
const glm::vec4 bullet_machinegun_colour = weapon_machinegun_colour;
// colours: vfx
const glm::vec4 player_splat_colour = player_colour;                             // player col
const glm::vec4 enemy_death_splat_colour = glm::vec4(0.65f, 0.65f, 0.65f, 1.0f); // greyish
const glm::vec4 enemy_impact_splat_colour = glm::vec4(0.95f, 0.3f, 0.3f, 1.0f);  // redish

static const std::vector<bool> GAME_COLL_MATRIX = {
  false, // NoCollision_NoCollision_0_0
  false, // bullet_NoCollision_1_0
  false, // Player_NoCollision_2_0
  false, // Enemy_NoCollision_3_0
  false, // Obstacle_NoCollision_4_0
  false, // Weapon_NoCollision_5_0

  false, // bullet_bullet_1_1
  false, // player_bullet_2_1
  true,  // enemy_bullet_3_1
  true,  // Obstacle_bullet_4_1
  false, // Weapon_bullet_5_1

  false, // player_player_2_2
  true,  // enemy_player_3_2
  true,  // Obstacle_player_4_2
  false, // Weapon_player_5_2

  false, // enemy_enemy_3_3
  true,  // Obstacle_enemy_4_3
  true,  // Weapon_enemy_5_3

  false, // Obstacle_Obstacle_4_4
  false, // Weapon_Obstacle_5_4

  false, // Weapon_Weapon_5_5
};
