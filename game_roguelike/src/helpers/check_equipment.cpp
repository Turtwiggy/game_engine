#include "check_equipment.hpp"

namespace game2d {

entt::entity
has_equipped(Game& game, const entt::entity& parent, const EquipmentSlot& slot)
{
  auto& r = game.state;
  for (auto [entity, equipped] : r.view<IsEquipped>().each()) {
    if (parent != equipped.parent)
      continue; // different parent
    if (equipped.slot == slot)
      return entity;
  }
  return entt::null;
};

} // namespace game2d