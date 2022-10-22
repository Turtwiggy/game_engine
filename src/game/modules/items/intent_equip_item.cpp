#include "intent_equip_item.hpp"

#include "components.hpp"
#include "game/modules/player/components.hpp"

namespace game2d {

void
update_intent_equip_item_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  {
    const auto& view = r.view<PlayerComponent, WantsToEquip>();
    for (const auto [entity, player, equip] : view.each()) {
      for (const auto& item : equip.items) {
        // process equipping items
        r.remove<InBackpackComponent>(item);
        r.emplace<IsEquipped>(item, entity);
      }
      r.remove<WantsToEquip>(entity);
    }
  }

  //
};

} // namespace game2d
