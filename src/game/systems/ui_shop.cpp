#include "ui_shop.hpp"

#include "game/create_entities.hpp"
#include "game/helpers/items.hpp"

#include <imgui.h>
#include <iostream> // temp
#include <memory>

namespace game2d {

void
update_ui_shop_system(entt::registry& r)
{
#ifdef _DEBUG
  bool show_imgui_demo_window = false;
  ImGui::ShowDemoWindow(&show_imgui_demo_window);
#endif

  ImGui::Begin("Shop");

  ImGui::Text("Potion");
  ImGui::SameLine();
  if (ImGui::Button("Buy##potion")) {
    const auto& view = r.view<PlayerComponent>();
    if (view.size() > 0) {
      auto main_player = view.front();
      if (auto* opt = r.try_get<InventoryComponent>(main_player)) {
        (*opt).inventory.push_back(std::make_shared<Potion>());
        std::cout << "buying potion" << std::endl;
      }
    }
  }

  ImGui::Text("Gun");
  ImGui::SameLine();
  if (ImGui::Button("Buy##gun")) {
    const auto& view = r.view<PlayerComponent>();
    if (view.size() > 0) {
      auto main_player = view.front();
      if (auto* opt = r.try_get<InventoryComponent>(main_player)) {
        (*opt).inventory.push_back(std::make_shared<Gun>());
        std::cout << "buying gun" << std::endl;
      }
    }
  }

  ImGui::End();
};

} // namespace game2d

// ImGui::Text("You have %i coin!", p0_currency);
//   if (ImGui::Button("Drain your coin..."))
//     p0_currency -= 1;

//     for (auto& icon : shop_item.second.icons) {
//       auto uv = convert_sprite_to_uv(icon);
//       ImGui::Image((ImTextureID)kenny_texture_id, { shop_icon_size.x, shop_icon_size.y }, uv[0], uv[1]);
//       ImGui::SameLine();
//     }

//     std::string wep = std::string(magic_enum::enum_name(shop_item.first));
//     if (shop_item.second.infinite_quantity)
//       ImGui::Text("Stock INF Price %i", shop_item.second.price);
//     else
//       ImGui::Text("Stock %i Price %i", shop_item.second.quantity, shop_item.second.price);

//     bool able_to_buy = p0_currency >= shop_item.second.price && shop_item.second.quantity > 0;
//     if (able_to_buy) {

//       ImGui::SameLine(ImGui::GetWindowWidth() - 40);
//       std::string buy_button_label = "Buy ##" + wep;

//       bool buy_button_clicked = ImGui::Button(buy_button_label.c_str());
//       if (buy_button_clicked) {
//         std::cout << "buy: " << wep << " clicked" << std::endl;

//         // reduce item quantity if not infinite
//         if (!shop_item.second.infinite_quantity)
//           shop_item.second.quantity -= 1;

//         // spend hard earned cash
//         p0_currency -= shop_item.second.price;

//         // shop logic
//         {
//           if (shop_item.first == ShopItem::PISTOL || shop_item.first == ShopItem::SHOTGUN ||
//               shop_item.first == ShopItem::MACHINEGUN) {
//             // hack: use player 0 for the moment
//             std::vector<ShopItem>& player_inv = player_inventories[0];
//             player_inv.push_back(shop_item.first);
//           }

//           if (shop_item.first == ShopItem::PISTOL_AMMO)
//             stats_pistol.current_ammo += shop_refill_pistol_ammo;
//           if (shop_item.first == ShopItem::SHOTGUN_AMMO)
//             stats_shotgun.current_ammo += shop_refill_shotgun_ammo;
//           if (shop_item.first == ShopItem::MACHINEGUN_AMMO)
//             stats_machinegun.current_ammo += shop_refill_machinegun_ammo;
