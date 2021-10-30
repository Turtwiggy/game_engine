#pragma once

// c++ headers
#include <map>

// game headers
#include "spritemap.hpp"

// game default states

constexpr int GAME_GRID_SIZE = 16;
constexpr int PIXELS_ON_SHEET = 16;
constexpr int PIXEL_SCALE_SIZE = 1;
static const int PIXELS_TO_RENDER = PIXELS_ON_SHEET;
constexpr int spritesheet_width = 768;
constexpr int spritesheet_height = 352;

constexpr int GAME_OVER_WAVE = 10;
constexpr int ENEMY_ATTACK_THRESHOLD = 4000;
constexpr bool SPAWN_ENEMIES = true;
constexpr int EXTRA_ENEMIES_TO_SPAWN_PER_WAVE = 5;
constexpr int SECONDS_UNTIL_MAX_DIFFICULTY = 100;
constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_START = 1.0f;
constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_END = 0.2f;

constexpr int ENEMY_BASE_HEALTH = 6;
constexpr int ENEMY_BASE_DAMAGE = 2;
constexpr int ENEMY_BASE_SPEED = 50;

constexpr float screenshake_time = 0.1f;
constexpr float vfx_flash_time = 0.2f;

// shop & stats
constexpr int shop_refill_pistol_ammo = 5;
constexpr int shop_refill_shotgun_ammo = 5;
constexpr int shop_refill_machinegun_ammo = 5;

// textures
constexpr int tex_unit_kenny_nl = 0;
constexpr int tex_unit_main_scene = 1;
constexpr int tex_unit_lighting = 2;

// sprites
constexpr game2d::sprite::type sprite_player = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_tree = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_weapon_base = game2d::sprite::type::WEAPON_SHOVEL;
constexpr game2d::sprite::type sprite_pistol = game2d::sprite::type::WEAPON_PISTOL;
constexpr game2d::sprite::type sprite_shotgun = game2d::sprite::type::WEAPON_SHOTGUN;
constexpr game2d::sprite::type sprite_machinegun = game2d::sprite::type::WEAPON_MP5;
constexpr game2d::sprite::type sprite_bullet = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_enemy_core = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_splat = game2d::sprite::type::SQUARE;
constexpr game2d::sprite::type sprite_heart_1 = game2d::sprite::type::ICON_HEART;
constexpr game2d::sprite::type sprite_heart_2 = game2d::sprite::type::ICON_HEART_OUTLINE;
constexpr game2d::sprite::type sprite_heart_3 = game2d::sprite::type::ICON_HEART_HALF_FULL;
constexpr game2d::sprite::type sprite_heart_4 = game2d::sprite::type::ICON_HEART_FULL;
constexpr game2d::sprite::type sprite_ammo = game2d::sprite::type::AMMO_BOX;

// colour palette; https://colorhunt.co/palette/273312
constexpr glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);  // black
constexpr glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f); // blue
constexpr glm::vec4 PALETTE_COLOUR_3_1 =
  glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
constexpr glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey
constexpr glm::vec4 PALETTE_COLOUR_5_1 = glm::vec4(0.95f, 0.3f, 0.3f, 1.0f);                                 // redish
// colours: entities
constexpr glm::vec4 background_colour = PALETTE_COLOUR_1_1; // black
constexpr glm::vec4 debug_line_colour = PALETTE_COLOUR_2_1; // blue
constexpr glm::vec4 player_colour = PALETTE_COLOUR_2_1;     // blue
constexpr glm::vec4 enemy_colour = PALETTE_COLOUR_5_1;      // red
// colours: weapons
constexpr glm::vec4 weapon_shovel_colour = PALETTE_COLOUR_3_1;
constexpr glm::vec4 weapon_shovel_flash_colour = PALETTE_COLOUR_2_1;
constexpr glm::vec4 weapon_pistol_colour = PALETTE_COLOUR_3_1;
constexpr glm::vec4 weapon_pistol_flash_colour = PALETTE_COLOUR_2_1;
constexpr glm::vec4 weapon_shotgun_colour = glm::vec4(1.0, 1.0, 0.0, 1.0);
constexpr glm::vec4 weapon_shotgun_flash_colour = PALETTE_COLOUR_2_1;
constexpr glm::vec4 weapon_machinegun_colour = PALETTE_COLOUR_3_1;
constexpr glm::vec4 weapon_machinegun_flash_colour = PALETTE_COLOUR_2_1;
constexpr glm::vec4 bullet_pistol_colour = weapon_pistol_colour;
constexpr glm::vec4 bullet_shotgun_colour = weapon_shotgun_colour;
constexpr glm::vec4 bullet_machinegun_colour = weapon_machinegun_colour;
// colours: vfx
constexpr glm::vec4 player_splat_colour = player_colour;                      // player col
constexpr glm::vec4 vfx_impact_colour = glm::vec4(0.65f, 0.65f, 0.65f, 1.0f); // greyish
constexpr glm::vec4 damage_number_colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // white
constexpr glm::vec4 enemy_death_splat_colour = PALETTE_COLOUR_5_1;            // same red
constexpr glm::vec4 enemy_impact_colour = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);  // brighter redish
