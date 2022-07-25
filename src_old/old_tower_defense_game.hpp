// #pragma once

// // game default states

// constexpr int GAME_OVER_WAVE = 10;
// constexpr int ENEMY_ATTACK_THRESHOLD = 4000;
// constexpr bool SPAWN_ENEMIES = true;
// constexpr int EXTRA_ENEMIES_TO_SPAWN_PER_WAVE = 5;
// constexpr int SECONDS_UNTIL_MAX_DIFFICULTY = 100;
// constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_START = 1.0f;
// constexpr float SECONDS_BETWEEN_SPAWNING_ENEMIES_END = 0.2f;

// constexpr int ENEMY_BASE_HEALTH = 6;
// constexpr int ENEMY_BASE_DAMAGE = 2;
// constexpr int ENEMY_BASE_SPEED = 50;

// constexpr float screenshake_time = 0.1f;
// constexpr float vfx_flash_time = 0.2f;

// // shop & stats
// constexpr int shop_refill_pistol_ammo = 5;
// constexpr int shop_refill_shotgun_ammo = 5;
// constexpr int shop_refill_machinegun_ammo = 5;

// // An "Attack" is basically a limiter that prevents collisions
// // applying damage on every frame. This could end up being super weird.
// struct Attack
// {
// private:
//   static inline uint32_t global_attack_int_counter = 0;

// public:
//   uint32_t id = 0;

//   int entity_weapon_owner_id; // player or enemy
//   int entity_weapon_id;
//   ShopItem weapon_type;
//   int weapon_damage = 0;

//   Attack(int parent, int weapon, ShopItem type, int damage)
//     : entity_weapon_owner_id(parent)
//     , entity_weapon_id(weapon)
//     , weapon_type(type)
//     , weapon_damage(damage)
//   {
//     id = ++Attack::global_attack_int_counter;
//   };
// };

// struct WeaponStats
// {
//   int damage = 0;
// };

// struct MeleeWeaponStats : WeaponStats
// {
//   // slash stats
//   float slash_attack_time = 0.15f;
//   float weapon_radius = 30.0f;
//   float weapon_angle_speed = engine::HALF_PI / 30.0f; // closer to 0 is faster
//   float weapon_damping = 20.0f;
//   float slash_attack_time_left = 0.0f;
//   bool attack_left_to_right = true;
//   glm::vec2 weapon_target_pos = { 0.0f, 0.0f };
//   float weapon_current_angle = 0.0f;
// };

// struct RangedWeaponStats : WeaponStats
// {
//   int radius_offset_from_player = 14;
//   bool infinite_ammo = true;
//   float fire_rate_seconds_limit = 1.0f;
//   int current_ammo = 20;
// };

// glm::ivec2
// player::rotate_b_around_a(const GameObject2D& a, const GameObject2D& b, float radius, float angle)
// {
//   const glm::vec2 center{ 0.0f, 0.0f };
//   const glm::vec2 offset = glm::vec2(center.x + radius * cos(angle), center.y + radius * sin(angle));

//   // Position b in a circle around the a, and render the b in it's center.
//   const glm::vec2 pos = convert_top_left_to_centre(a);
//   const glm::vec2 rot_pos = pos + offset - (glm::vec2(b.physics_size) / 2.0f);

//   return glm::ivec2(int(rot_pos.x), int(rot_pos.y));
// };

// void
// shop::update_shop(int& p0_currency,
//                   int kenny_texture_id,
//                   std::map<ShopItem, ShopItemState>& shop,
//                   RangedWeaponStats& stats_pistol,
//                   RangedWeaponStats& stats_shotgun,
//                   RangedWeaponStats& stats_machinegun,
//                   int shop_refill_pistol_ammo,
//                   int shop_refill_shotgun_ammo,
//                   int shop_refill_machinegun_ammo,
//                   std::vector<std::vector<ShopItem>>& player_inventories,
//                   std::vector<GameObject2D>& entities_player)
// {
//   ImGui::Text("You have %i coin!", p0_currency);

//   if (ImGui::Button("Drain your coin..."))
//     p0_currency -= 1;

//   const glm::vec2 shop_icon_size = { 20.0f, 20.0f };
//   for (auto& shop_item : shop) {

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

//           if (shop_item.first == ShopItem::HEAL_HALF) {
//             GameObject2D& p0 = entities_player[0];
//             p0.damage_taken -= static_cast<int>(p0.damage_able_to_be_taken / 2);
//             if (p0.damage_taken < 0)
//               p0.damage_taken = 0;
//           }

//           if (shop_item.first == ShopItem::HEAL_FULL) {
//             GameObject2D& p0 = entities_player[0];
//             p0.damage_taken = 0;
//           }
//         }
//       }
//     }
//   }
// }

// // your header
// #include "player.hpp"

// void
// position_around_player(GameObject2D& player, GameObject2D& weapon, RangedWeaponStats& stats, KeysAndState& keys)
// {
//   weapon.do_render = true;
//   weapon.angle_radians = keys.angle_around_player;

//   const int radius = stats.radius_offset_from_player;
//   const float angle = keys.angle_around_player;
//   glm::vec2 rot_pos = player::rotate_b_around_a(player, weapon, float(radius), angle);
//   weapon.pos = rot_pos;
// };

// // header
// #include "enemy_spawner.hpp"

// // other project headers
// #include <glm/gtx/norm.hpp>
