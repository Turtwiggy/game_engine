#include "intent_use_item.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_intent_use_item_system(entt::registry& r)
{
  //
  // WantsToUse
  const auto& use_view = r.view<WantsToUse>();
  use_view.each([&r](auto entity, auto& intent) {
    //
    for (int i = 0; i < intent.items.size(); i++) {
      Use u = intent.items[i];
      auto* item_heals = r.try_get<GiveHealsComponent>(u.item);
      auto* item_consumable = r.try_get<ConsumableComponent>(u.item);

      // If not, use on self
      // auto& target = entity;
      // auto* target_hp = r.try_get<HealthComponent>(entity);

      // If targets are set, use those
      for (int j = 0; j < u.targets.size(); j++) {
        auto& target = u.targets[j];
        auto* target_hp = r.try_get<HealthComponent>(target);

        // Does the item heal?
        if (item_heals && target_hp) {
          target_hp->hp = glm::min(target_hp->max_hp, target_hp->hp + item_heals->health);
          std::cout << "hp now at: " << target_hp->hp << "\n";
        }
      }

      // Is the item consumable?
      if (item_consumable)
        r.destroy(u.item);
    }

    // Done processing all the items
    r.remove<WantsToUse>(entity);
  });

  // WantsToDrop
  const auto& drop_view = r.view<const TransformComponent, WantsToDrop>();
  drop_view.each([&r](auto entity, const auto& actor, auto& intent) {
    //
    for (int i = 0; i < intent.items.size(); i++) {
      entt::entity item = intent.items[i];

      create_renderable(r, item, ENTITY_TYPE::POTION);
      auto& transform = r.get<TransformComponent>(item);
      transform.position.x = actor.position.x;
      transform.position.y = actor.position.y;

      r.remove<InBackpackComponent>(item);
    }
    r.remove<WantsToDrop>(entity);
  });

  // WantsToPurchase
  const auto& purchase_view = r.view<const WantsToPurchase>();
  purchase_view.each([&r](auto entity, auto& intent) {
    for (int i = 0; i < intent.items.size(); i++) {
      entt::entity item = intent.items[i];
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

    for (int i = 0; i < intent.items.size(); i++) {
      entt::entity item = intent.items[i];
      if (auto* backpack = r.try_get<InBackpackComponent>(item)) {
        // remove from the entity regretful hands
        // why is the entity regretful? i dunno man
        // put it in the now happy again shopkeeper
        backpack->parent = shopkeeper0;
      }
    }

    r.remove<WantsToSell>(entity); // assume success
  });

  // WantsToAttack
  // ...
};

} // namespace game2d