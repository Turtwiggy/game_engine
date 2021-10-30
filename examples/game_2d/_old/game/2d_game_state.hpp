#pragma once

// c++ lib headers
#include <map>
#include <vector>

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths.hpp"

// game headers
#include "2d_game_object.hpp"
#include "2d_lighting.hpp"
#include "2d_physics.hpp"
#include "constants.hpp"
#include "items.hpp"
#include "player_keys.hpp"
#include "shop.hpp"
#include "spritemap.hpp"

namespace game2d {

struct MutableGameState
{
  GameRunning game_running = GameRunning::ACTIVE;
  GamePhase game_phase = GamePhase::ATTACK;
  EditorMode editor_left_click_mode = EditorMode::PLAYER_ATTACK;

  std::map<ShopItem, ShopItemState> shop = shop::shop_initial_state();
  int p0_currency = 0;

  int enemies_destroyed_this_wave = 0;
  int enemies_killed = 0;

  MeleeWeaponStats stats_shovel = create_shovel();
  RangedWeaponStats stats_pistol = create_pistol();
  RangedWeaponStats stats_shotgun = create_shotgun();
  RangedWeaponStats stats_machinegun = create_machinegun();

  GameObject2D camera = GameObject2D();
  GameObject2D weapon_shovel = gameobject::create_weapon(sprite_weapon_base, tex_unit_kenny_nl, weapon_shovel_colour);
  GameObject2D weapon_pistol = gameobject::create_weapon(sprite_pistol, tex_unit_kenny_nl, weapon_pistol_colour);
  GameObject2D weapon_shotgun = gameobject::create_weapon(sprite_shotgun, tex_unit_kenny_nl, weapon_shotgun_colour);
  GameObject2D weapon_machinegun =
    gameobject::create_weapon(sprite_machinegun, tex_unit_kenny_nl, weapon_machinegun_colour);

  // spawn vars
  int wave = 0;
  // difficulty: spawn amount
  int enemies_to_spawn_this_wave = 10;
  int enemies_to_spawn_this_wave_left = enemies_to_spawn_this_wave;
  // difficulty: spawn rate
  float game_seconds_until_max_difficulty_spent = 0.0f;
  float game_enemy_seconds_between_spawning_current = SECONDS_BETWEEN_SPAWNING_ENEMIES_START;
  float game_enemy_seconds_between_spawning_left = 0.0f;

  // all entities
  std::vector<Attack> attacks;
  std::vector<CollisionEvent> collision_events;
  std::vector<GameObject2D> entities_bullets;
  std::vector<GameObject2D> entities_enemies;
  std::vector<GameObject2D> entities_player;
  std::vector<GameObject2D> entities_shops;
  std::vector<GameObject2D> entities_trees;
  std::vector<GameObject2D> entities_vfx;
  std::vector<KeysAndState> player_keys;
  std::vector<std::vector<ShopItem>> player_inventories;
  std::vector<PointLight> point_lights;

  entt::registry registry;
};

}; // namespace game2d