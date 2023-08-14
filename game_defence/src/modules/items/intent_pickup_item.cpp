#include "intent_pickup_item.hpp"

#include "components.hpp"
#include "modules/physics/components.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"

#include <iostream>

namespace game2d {

void
update_intent_pickup_system(entt::registry& r)
{

  // update pickup zone positions...
  for (const auto& [entity, player, player_t] : r.view<const PlayerComponent, const TransformComponent>().each()) {
    const auto& zone = player.pickup_area;
    auto& zone_transform = r.get<TransformComponent>(zone);
    zone_transform.position = player_t.position;
  }

  for (const auto& [entity, request] : r.view<WantsToPickUp>().each()) {

    // check the circle fixture around the player
    // if the circle fixture is colliding with any entities,
    // if able to be picked up,
    // pick them up

    std::cout << "done with pickup intent" << std::endl;
    r.remove<WantsToPickUp>(entity);
  }

  // // WantsToPurchase
  // const auto& purchase_view = r.view<const WantsToPurchase>();
  // for (auto [entity, intent] : purchase_view.each()) {
  //   for (const entt::entity& item : intent.items) {

  //     auto* backpack = r.try_get<InBackpackComponent>(item);
  //     if (backpack) {

  //       // was the item in the shopkeepers hands?
  //       // was the item in another player's hands?
  //       // should probably validate who the item is being removed from

  //       // remove from someone's angry hands
  //       // why are they angry? i dunno man, they're being robbed
  //       // put it in the entity hands
  //       backpack->parent = entity;
  //     }

  //     // If it wasn't in a backpack, put it in a backpack
  //     // this can occur if the item was picked up from the floor
  //     // should the world have a backpack??
  //     else {
  //       InBackpackComponent& new_backpack = r.emplace<InBackpackComponent>(item);
  //       new_backpack.parent = entity;
  //     }
  //   }

  //   // Done processing the item
  //   r.remove<WantsToPurchase>(entity);
  // }

  // // WantsToSell
  // // const auto& sell_view = r.view<const WantsToSell>();
  // // sell_view.each([&r](auto entity, auto& intent) {
  // //   //
  // //   // Hack: sell to the first shopkeeper for the moment
  // //   const auto& shopkeepers = r.view<ShopKeeperComponent>();
  // //   const auto& shopkeeper0 = shopkeepers.front();
  // //   for (const auto& item : intent.items) {
  // //     if (auto* backpack = r.try_get<InBackpackComponent>(item)) {
  // //       // remove from the entity regretful hands
  // //       // why is the entity regretful? i dunno man
  // //       // put it in the now happy again shopkeeper
  // //       backpack->parent = shopkeeper0;
  // //     }
  // //   }
  // //   r.remove<WantsToSell>(entity); // assume success
  // // });
}

} // namespace game2d
