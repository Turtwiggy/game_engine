#pragma once

#include <entt/entt.hpp>

#include <memory>
#include <string>
#include <vector>

namespace game2d {

struct Item
{
  std::string name;
  bool infinite_quantity = true;
  int quantity = 0;
  int cost = 0;

  // C++ Core Guidelines: C.67
  // A polymorphic class should suppress public copy/move.
  // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c67-a-polymorphic-class-should-suppress-public-copymove
  Item() = default;
  Item(const Item&) = delete;
  Item& operator=(const Item&) = delete;

  virtual bool use(entt::registry& r, std::vector<entt::entity>& entities) { return false; };
};

struct Potion : public Item
{
  int heal_amount = 1;

  Potion();
  virtual bool use(entt::registry& r, std::vector<entt::entity>& entities);
};

struct SINGLETON_ShopComponent
{
  std::vector<std::shared_ptr<Item>> available_items;
};

struct InventoryComponent
{
  std::vector<std::shared_ptr<Item>> inventory;
};

} // namespace game2d