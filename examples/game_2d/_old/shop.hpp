#pragma once

// c++ headers
#include <map>
#include <vector>

// your project headers
#include "2d_game_object.hpp"
#include "spritemap.hpp"
#include "weapons.hpp"

namespace game2d {

struct ShopItemState
{
  std::vector<sprite::type> icons;

  bool free = false;
  int price = 10;
  bool infinite_quantity = false;
  int quantity = 1;
};

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

} // namespace shop

} // namespace game2d