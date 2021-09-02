#pragma once

// c++ lib headers
#include <vector>

// other lib headers
#include <glm/glm.hpp>

// fightingengine headers
#include "engine/application.hpp"
#include "engine/maths_core.hpp"

// game headers
#include "2d_game_config.hpp"
#include "2d_game_object.hpp"
#include "2d_lighting.hpp"
#include "2d_physics.hpp"
#include "spritemap.hpp"

namespace game2d {

struct MutableGameState;

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
next_wave(int& enemies_to_spawn_this_wave, int& enemies_to_spawn_this_wave_left, int& wave);

// spawn a random enemy every X seconds
void
update(MutableGameState& state, fightingengine::RandomState& rnd, const glm::ivec2 screen_wh, const float delta_time_s);

}; // namespace enemy_spawner

namespace player {

glm::ivec2
rotate_b_around_a(const GameObject2D& a, const GameObject2D& b, float radius, float angle);

void
scroll_to_swap_weapons(fightingengine::Application& app, GameObject2D& player, std::vector<ShopItem>& inventory);

void
update_input(GameObject2D& obj, KeysAndState& keys, fightingengine::Application& app, GameObject2D& camera);

void
ability_boost(GameObject2D& player, const KeysAndState& keys, const float delta_time_s);

void
ability_slash(fightingengine::Application& app,
              GameObject2D& player_obj,
              KeysAndState& keys,
              GameObject2D& weapon,
              float delta_time_s,
              MeleeWeaponStats& s,
              std::vector<Attack>& attacks);

void
player_attack(fightingengine::Application& app,
              MutableGameState& gs,
              GameObject2D& player,
              std::vector<ShopItem>& player_inventory,
              KeysAndState& keys,
              const float delta_time_s,
              fightingengine::RandomState& rnd);

}; // namespace player

namespace shop {

[[nodiscard]] std::map<ShopItem, ShopItemState>
shop_initial_state();

void
update_shop(int& p0_currency,
            int kenny_texture_id,
            std::map<ShopItem, ShopItemState>& shop,
            RangedWeaponStats& stats_pistol,
            RangedWeaponStats& stats_shotgun,
            RangedWeaponStats& stats_machinegun,
            int shop_refill_pistol_ammo,
            int shop_refill_shotgun_ammo,
            int shop_refill_machinegun_ammo,
            std::vector<std::vector<ShopItem>>& player_inventories,
            std::vector<GameObject2D>& entities_player);

}; // namespace shop

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
};

}; // namespace game2d