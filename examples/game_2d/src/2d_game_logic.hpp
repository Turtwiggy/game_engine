#pragma once

// c++ lib headers
#include <vector>

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

void
next_wave();

[[nodiscard]] int
get_wave();

[[nodiscard]] int
enemies_left_to_spawn();

void
spawn_enemy(std::vector<GameObject2D>& enemies, fightingengine::RandomState& rnd, glm::vec2 world_pos);

// spawn a random enemy every X seconds
void
update(std::vector<GameObject2D>& enemies,
       std::vector<GameObject2D>& players,
       const GameObject2D& camera,
       fightingengine::RandomState& rnd,
       const glm::ivec2 screen_wh,
       const float delta_time_s);

}; // namespace enemy_spawner

namespace player {

void
update_input(GameObject2D& obj, KeysAndState& keys, fightingengine::Application& app, GameObject2D& camera);

void
ability_boost(GameObject2D& player, const KeysAndState& keys, const float delta_time_s);

void
ability_slash(fightingengine::Application& app,
              GameObject2D& player_obj,
              const KeysAndState& keys,
              GameObject2D& weapon,
              float delta_time_s,
              MeleeWeaponStats& s,
              std::vector<Attack>& attacks);

void
ability_shoot(GameObject2D& entity_to_fire_from,
              const KeysAndState& keys,
              std::vector<GameObject2D>& bullets,
              const glm::vec4 bullet_col,
              const sprite::type sprite,
              RangedWeaponStats& s,
              std::vector<Attack>& attacks);

void
player_attack(fightingengine::Application& app,
              GameObject2D& player,
              MeleeWeaponStats& stats_shovel,
              RangedWeaponStats& stats_pistol,
              RangedWeaponStats& stats_shotgun,
              RangedWeaponStats& stats_machinegun,
              GameObject2D& weapon_shovel,
              GameObject2D& weapon_pistol,
              GameObject2D& weapon_shotgun,
              GameObject2D& weapon_machinegun,
              const std::vector<ShopItem>& player_inventory,
              const float delta_time_s,
              const KeysAndState& keys,
              std::vector<Attack>& attacks,
              std::vector<GameObject2D>& entities_bullets);

}; // namespace player

namespace shop {

struct ShopItemState
{
  bool free = false;
  int price = 10;

  bool infinite_quantity = false;
  int quantity = 1;
};

[[nodiscard]] std::map<ShopItem, ShopItemState>
shop_initial_state();

void
update_shop(int& p0_currency,
            std::map<ShopItem, shop::ShopItemState>& shop,
            std::vector<std::vector<ShopItem>>& player_inventories,
            std::vector<GameObject2D>& entities_player);

} // namespace shop

}; // namespace game2d