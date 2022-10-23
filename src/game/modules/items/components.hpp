#pragma once

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <vector>

//
// item/inventory
//

namespace game2d {

struct ShopKeeperComponent
{
  bool placeholder = true;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShopKeeperComponent, placeholder);
};

struct AbleToBePickedUp
{
  bool placeholder = true;
};

struct InBackpackComponent
{
  entt::entity parent = entt::null;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(InBackpackComponent, parent);
};

struct ConsumableComponent
{
  int uses_left = 1;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConsumableComponent, uses_left);
};

struct GiveHealsComponent
{
  int health = 1;
  // std::vector<int> health_q;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GiveHealsComponent, health);
};

struct TakeHealsComponent
{
  std::vector<int> heals;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TakeHealsComponent, heals);
};

// "intent" components surrounding items
struct Use
{
  entt::entity entity = entt::null;
  std::vector<entt::entity> targets;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Use, entity, targets);
};
struct WantsToUse
{
  std::vector<Use> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToUse, items);
};
struct WantsToDrop
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToDrop, items);
};

struct WantsToPurchase
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToPurchase, items);
};
struct WantsToSell
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToSell, items);
};

struct WantsToSelectUnitsForItem
{
  std::vector<Use> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToSelectUnitsForItem, items);
};

//
// equipment
//

enum class EquipmentSlot
{
  left_hand,
  right_hand,
};

struct Equipment
{
  bool placeholder = true;
};

struct IsEquipped
{
  EquipmentSlot slot;
  entt::entity parent = entt::null;
};

struct EquipmentRequest
{
  EquipmentSlot slot;
  entt::entity item;
};

struct WantsToEquip
{
  std::vector<EquipmentRequest> requests;
};

} // namespace game2d