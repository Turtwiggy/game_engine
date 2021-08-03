#pragma once

#include "2d_game_object.hpp"
#include "spritemap.hpp"

// textures
const int tex_unit_kenny_nl = 0;
const int tex_unit_tree = 1;

// sprites
const game2d::sprite::type sprite_player = game2d::sprite::type::PERSON_1;
const game2d::sprite::type sprite_pistol = game2d::sprite::type::WEAPON_PISTOL;
const game2d::sprite::type sprite_shotgun = game2d::sprite::type::WEAPON_SHOTGUN;
const game2d::sprite::type sprite_machinegun = game2d::sprite::type::WEAPON_MP5;
const game2d::sprite::type sprite_bullet = game2d::sprite::type::TREE_1;
const game2d::sprite::type sprite_enemy_core = game2d::sprite::type::PERSON_2;
const game2d::sprite::type sprite_weapon_base = game2d::sprite::type::WEAPON_SHOVEL;
const game2d::sprite::type sprite_splat = game2d::sprite::type::CASTLE_FLOOR;

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
