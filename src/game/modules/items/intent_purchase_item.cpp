#include "intent_purchase_item.hpp"

#include "components.hpp"

namespace game2d {

void
update_intent_purchase_item(GameEditor& editor, Game& game)
{
  auto& r = game.state;

  // WantsToPurchase
  const auto& purchase_view = r.view<const WantsToPurchase>();
  purchase_view.each([&r](auto entity, auto& intent) {
    for (const auto& item : intent.items) {
      if (auto* backpack = r.try_get<InBackpackComponent>(item)) {
        // remove from the shopkeepers angry hands
        // why is the shop keeper angry? i dunno man
        // put it in the entity hands
        backpack->parent = entity;
      }
    }
    r.remove<WantsToPurchase>(entity); // assume success
  });

  // WantsToSell
  const auto& sell_view = r.view<const WantsToSell>();
  sell_view.each([&r](auto entity, auto& intent) {
    //
    // Hack: sell to the first shopkeeper for the moment
    const auto& shopkeepers = r.view<ShopKeeperComponent>();
    const auto& shopkeeper0 = shopkeepers.front();

    for (const auto& item : intent.items) {
      if (auto* backpack = r.try_get<InBackpackComponent>(item)) {
        // remove from the entity regretful hands
        // why is the entity regretful? i dunno man
        // put it in the now happy again shopkeeper
        backpack->parent = shopkeeper0;
      }
    }

    r.remove<WantsToSell>(entity); // assume success
  });
}

} // namespace game2d
