// header
#include "shop.hpp"

// c++ headers
#include <iostream>
#include <map>

// other project headers
#include "thirdparty/magic_enum.hpp"

namespace game2d {

std::map<ShopItem, ShopItemState>
shop::shop_initial_state()
{
  // configure shop
  std::map<ShopItem, ShopItemState> shop;
  {
    {
      ShopItemState i;
      i.price = 10;
      i.quantity = 1;
      i.infinite_quantity = false;
      i.icons.push_back(sprite_pistol);
      shop[ShopItem::PISTOL] = i;
    };
    {
      ShopItemState i;
      i.price = 3;
      i.infinite_quantity = true;
      i.icons.push_back(sprite_ammo);
      shop[ShopItem::PISTOL_AMMO] = i;
    };
    {
      ShopItemState i;
      i.price = 20;
      i.quantity = 1;
      i.infinite_quantity = false;
      i.icons.push_back(sprite_shotgun);
      shop[ShopItem::SHOTGUN] = i;
    };
    {
      ShopItemState i;
      i.price = 6;
      i.infinite_quantity = true;
      i.icons.push_back(sprite_ammo);
      shop[ShopItem::SHOTGUN_AMMO] = i;
    };
    {
      ShopItemState i;
      i.price = 40;
      i.quantity = 1;
      i.infinite_quantity = false;
      i.icons.push_back(sprite_machinegun);
      shop[ShopItem::MACHINEGUN] = i;
    };
    {
      ShopItemState i;
      i.price = 5;
      i.infinite_quantity = true;
      i.icons.push_back(sprite_ammo);
      shop[ShopItem::MACHINEGUN_AMMO] = i;
    };
    {
      ShopItemState i;
      i.price = 40;
      i.infinite_quantity = true;
      i.icons.push_back(sprite_heart_3);
      shop[ShopItem::HEAL_HALF] = i;
    };
    {
      ShopItemState i;
      i.price = 80;
      i.infinite_quantity = true;
      i.icons.push_back(sprite_heart_4);
      shop[ShopItem::HEAL_FULL] = i;
    };
  };
  return shop;
}

void
shop::update_shop(int& p0_currency,
                  int kenny_texture_id,
                  std::map<ShopItem, ShopItemState>& shop,
                  RangedWeaponStats& stats_pistol,
                  RangedWeaponStats& stats_shotgun,
                  RangedWeaponStats& stats_machinegun,
                  int shop_refill_pistol_ammo,
                  int shop_refill_shotgun_ammo,
                  int shop_refill_machinegun_ammo,
                  std::vector<std::vector<ShopItem>>& player_inventories,
                  std::vector<GameObject2D>& entities_player)
{
  ImGui::Text("You have %i coin!", p0_currency);

  if (ImGui::Button("Drain your coin..."))
    p0_currency -= 1;

  const glm::vec2 shop_icon_size = { 20.0f, 20.0f };
  for (auto& shop_item : shop) {

    for (auto& icon : shop_item.second.icons) {
      auto uv = convert_sprite_to_uv(icon);
      ImGui::Image((ImTextureID)kenny_texture_id, { shop_icon_size.x, shop_icon_size.y }, uv[0], uv[1]);
      ImGui::SameLine();
    }

    std::string wep = std::string(magic_enum::enum_name(shop_item.first));

    if (shop_item.second.infinite_quantity)
      ImGui::Text("Stock INF Price %i", shop_item.second.price);
    else
      ImGui::Text("Stock %i Price %i", shop_item.second.quantity, shop_item.second.price);

    bool able_to_buy = p0_currency >= shop_item.second.price && shop_item.second.quantity > 0;
    if (able_to_buy) {

      ImGui::SameLine(ImGui::GetWindowWidth() - 40);
      std::string buy_button_label = "Buy ##" + wep;
      bool buy_button_clicked = ImGui::Button(buy_button_label.c_str());
      if (buy_button_clicked) {
        std::cout << "buy: " << wep << " clicked" << std::endl;

        // reduce item quantity if not infinite
        if (!shop_item.second.infinite_quantity)
          shop_item.second.quantity -= 1;

        // spend hard earned cash
        p0_currency -= shop_item.second.price;

        // shop logic
        {
          if (shop_item.first == ShopItem::PISTOL || shop_item.first == ShopItem::SHOTGUN ||
              shop_item.first == ShopItem::MACHINEGUN) {
            // hack: use player 0 for the moment
            std::vector<ShopItem>& player_inv = player_inventories[0];
            player_inv.push_back(shop_item.first);
          }

          if (shop_item.first == ShopItem::PISTOL_AMMO)
            stats_pistol.current_ammo += shop_refill_pistol_ammo;
          if (shop_item.first == ShopItem::SHOTGUN_AMMO)
            stats_shotgun.current_ammo += shop_refill_shotgun_ammo;
          if (shop_item.first == ShopItem::MACHINEGUN_AMMO)
            stats_machinegun.current_ammo += shop_refill_machinegun_ammo;

          if (shop_item.first == ShopItem::HEAL_HALF) {
            GameObject2D& p0 = entities_player[0];
            p0.damage_taken -= static_cast<int>(p0.damage_able_to_be_taken / 2);
            if (p0.damage_taken < 0)
              p0.damage_taken = 0;
          }

          if (shop_item.first == ShopItem::HEAL_FULL) {
            GameObject2D& p0 = entities_player[0];
            p0.damage_taken = 0;
          }
        }
      }
    }
  }
}

} // namespace game2d