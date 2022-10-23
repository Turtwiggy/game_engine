#include "intent_equip_item.hpp"

#include "components.hpp"
#include "game/helpers/check_equipment.hpp"
#include "game/modules/player/components.hpp"

namespace game2d {

void
update_intent_equip_item_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  const auto& view = r.view<PlayerComponent, WantsToEquip>();
  for (const auto [entity, player, equip] : view.each()) {

    for (const auto& request : equip.requests) {
      const auto& item = request.item;
      const auto& slot = request.slot;

      const auto& equipped_item = has_equipped(game, entity, slot);
      if (equipped_item != entt::null) {
        // unequip the equipped item
        r.remove<IsEquipped>(equipped_item);
        InBackpackComponent backpack_item;
        backpack_item.parent = entity;
        r.emplace<InBackpackComponent>(equipped_item, backpack_item);
      }

      // equip the new item
      r.remove<InBackpackComponent>(item);
      IsEquipped equipped;
      equipped.slot = slot;
      equipped.parent = entity;
      r.emplace<IsEquipped>(item, equipped);
    }

    r.remove<WantsToEquip>(entity);
  }
};

} // namespace game2d
