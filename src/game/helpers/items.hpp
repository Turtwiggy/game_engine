#pragma once

#include <entt/entt.hpp>

#include <memory>
#include <string>
#include <vector>

namespace game2d {

struct Equipment
{
  std::string name;
  bool destroy_after_use = false;
  int count = 0; // stack equipment

  // C++ Core Guidelines: C.67
  // A polymorphic class should suppress public copy/move.
  // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c67-a-polymorphic-class-should-suppress-public-copymove
  Equipment() = default;
  Equipment(const Equipment&) = delete;
  Equipment& operator=(const Equipment&) = delete;

  virtual bool use(entt::registry& r, std::vector<entt::entity>& entities) { return false; };
};

struct Potion : public Equipment
{
  int use_cooldown = 1;
  int heal_amount = 1;

  Potion()
  {
    name = "default-potion";
    destroy_after_use = true;
  };
  virtual bool use(entt::registry& r, std::vector<entt::entity>& entities);
};

// todo: implement pistol and shotgun as a type of gun
// just to see what to do with them
struct Gun : public Equipment
{
  int use_cooldown = 1;
  int damage = 1;
  int projectiles = 1;
  int ammo = 1;
  float fire_rate = 1.0f;
  bool infinite_ammo = false;

  Gun() { name = "default-gun"; };
  virtual bool use(entt::registry& r, std::vector<entt::entity>& entities);
};

struct Sword : public Equipment
{
  int use_cooldown = 1;
  int damage = 1;
  int weapon_radius = 30;
  float attack_rate = 0.15f;

  Sword() { name = "default-sword"; };
  virtual bool use(entt::registry& r, std::vector<entt::entity>& entities);
};

struct InventoryComponent
{
  std::vector<std::shared_ptr<Equipment>> inventory;
};

//
// SHOP
//

// struct ShopItem
// {
//   Equipment item;
//   bool infinite_quantity = true;
//   int quantity = 0;
//   int cost = 0;
// };

struct SINGLETON_ShopComponent
{
  // std::vector<std::shared_ptr<ShopItem>> available_items;
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