#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <iostream>
#include <optional>
#include <vector>

namespace game2d {

std::optional<entt::entity>
check_zone_and_player_collision(entt::registry& r, const SINGLETON_PhysicsComponent& physics, const entt::entity& zone_e)
{
  // no collison between zone <= => pickup entity
  // sc.colour.r = .0f;

  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    // collision between pickup zone <= => pickup entity
    //
    const auto a_other = r.try_get<WantsToPickUp>(a);
    const auto b_other = r.try_get<WantsToPickUp>(b);
    if (a == zone_e && b_other)
      return b;
    if (b == zone_e && a_other)
      return a;
  }

  return std::nullopt;
}

void
update_intent_pickup_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  const auto& items_view = r.view<HasParentComponent, ItemComponent>(entt::exclude<WaitForInitComponent>);

  const auto& pickup_view = r.view<PickupZoneComponent, const AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [zone_e, pickup, aabb] : pickup_view.each()) {

    const auto other_e = check_zone_and_player_collision(r, physics, zone_e);
    if (other_e == std::nullopt)
      continue;

    // limit amount of items a player can have picked up
    bool hit_limit = false;
    if (auto* limit = r.try_get<InventoryLimit>(other_e.value())) {
      int items = 0;
      for (const auto& [item_e, parent, item] : items_view.each()) {
        const bool item_belongs_to_entity = parent.parent == other_e;
        if (item_belongs_to_entity)
          items++;
      }
      hit_limit = items >= limit->amount;
    }

    if (!hit_limit) {
      // give the entity an item
      const auto req = create_gameplay(r, EntityType::item);
      r.emplace_or_replace<HasParentComponent>(req, other_e.value());
      r.get<ItemComponent>(req).item_id = pickup.spawn_item_with_id;
      r.remove<TransformComponent>(req);
    }
  }

  const auto& dropoff_view = r.view<DropoffZoneComponent, const AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [zone_e, zone, aabb] : dropoff_view.each()) {

    const auto other_e = check_zone_and_player_collision(r, physics, zone_e);
    if (other_e == std::nullopt)
      continue;
    // there is a something standing in the dropoff zone that wanted to pick something up

    // remove an item from the entity
    // (assuming valid criteria are met)
    //
    auto& zone_wanted_items = zone.requested_items;

    for (const auto& [item_e, parent, item] : items_view.each()) {
      const auto& item_id = item.item_id;

      auto item_wanted_by_dropzone_it = std::find(zone_wanted_items.begin(), zone_wanted_items.end(), item_id);
      const bool item_wanted_by_dropzone = item_wanted_by_dropzone_it != zone_wanted_items.end();
      const bool item_belongs_to_entity = parent.parent == other_e;

      if (item_belongs_to_entity && item_wanted_by_dropzone) {
        // accept the item
        r.destroy(item_e);

        // (gameplay descision)
        // remove item from the request list
        zone_wanted_items.erase(item_wanted_by_dropzone_it);

        break; // one at a time
      }
    }
  }

  // done with requests
  const auto& reqs = r.view<WantsToPickUp>(entt::exclude<WaitForInitComponent>);
  r.remove<WantsToPickUp>(reqs.begin(), reqs.end());
}

} // namespace game2d
