#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include <iostream>
#include <optional>

namespace game2d {

std::optional<entt::entity>
check_zone_and_player_collision(entt::registry& r,
                                const SINGLETON_PhysicsComponent& physics,
                                const entt::entity& zone_e,
                                SpriteColourComponent& colour)
{
  // no collison between zone <= => player
  colour.colour->r = 0.0f;

  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    // collision between pickup zone <= => player
    //
    const auto a_player = r.try_get<PlayerComponent>(a);
    const auto b_player = r.try_get<PlayerComponent>(b);
    if (a == zone_e && b_player) {
      colour.colour->r = 1.0f;

      // player and zone are colliding...
      if (const auto* pickup = r.try_get<WantsToPickUp>(b))
        return b;
    }
    if (b == zone_e && a_player) {
      colour.colour->r = 1.0f;

      // player and zone are colliding...
      if (const auto* pickup = r.try_get<WantsToPickUp>(a))
        return a;
    }
  }

  return std::nullopt;
}

void
update_intent_pickup_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  const auto& reqs = r.view<WantsToPickUp>(entt::exclude<WaitForInitComponent>);

  const auto& pickup_zones =
    r.view<PickupZoneComponent, const AABB, SpriteColourComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [zone_e, pickup, aabb, colour] : pickup_zones.each()) {

    const auto other_e = check_zone_and_player_collision(r, physics, zone_e, colour);
    if (other_e == std::nullopt)
      continue;

    // give the entity an item
    //
    const auto req = create_gameplay(r, EntityType::item);
    r.get<ItemComponent>(req).item_id = pickup.spawn_item_with_id;

    HasParentComponent hpc;
    hpc.parent = other_e.value();
    r.emplace_or_replace<HasParentComponent>(req, hpc);
  }

  const auto& dropoff_zones =
    r.view<DropoffZoneComponent, const AABB, SpriteColourComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [zone_e, zone, aabb, colour] : dropoff_zones.each()) {

    const auto other_e = check_zone_and_player_collision(r, physics, zone_e, colour);
    if (other_e == std::nullopt)
      continue;

    // remove an item from the entity
    //
    const auto items_view = r.view<HasParentComponent, ItemComponent>(entt::exclude<WaitForInitComponent>);
    for (const auto& [item_e, parent, item] : items_view.each()) {
      if (parent.parent == other_e) {
        std::cout << "dropping off item..." << std::endl;
        r.destroy(item_e);
        break; // drop one at a time
      }
    }
  }

  // done with requests
  r.remove<WantsToPickUp>(reqs.begin(), reqs.end());
}

} // namespace game2d
