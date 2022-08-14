#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

struct InventoryComponent
{
  std::vector<entt::entity> inventory;
};

//
// SHOP
//

// struct ShopItem
// {
//   std::shared_ptr<Equipment> item;
//   bool infinite_quantity = true;
//   int quantity = 0;
//   int cost = 0;
// };

struct SINGLETON_ShopComponent
{
  // std::vector<ShopItem> items;
  bool placeholder = true;
};

} // namespace game2d

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