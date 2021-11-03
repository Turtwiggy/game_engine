#pragma once

// game default states

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