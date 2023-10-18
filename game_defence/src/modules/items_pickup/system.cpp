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

namespace game2d {

void
update_intent_pickup_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& zones = r.view<PickupZoneComponent, HasParentComponent, AABB>();

  for (const auto& [entity, zone, parent, aabb] : zones.each()) {
    const auto& player = parent.parent;
    const auto& player_aabb = r.get<AABB>(player);
    auto& player_comp = r.get<PlayerComponent>(player);

    // update pickup zone positions for collisions next frmae
    aabb.center = player_aabb.center;

    // check for pickup collisions
    auto* wants_to_pickup_request = r.try_get<WantsToPickUp>(player);
    if (!wants_to_pickup_request)
      continue;
    r.remove<WantsToPickUp>(player); // processed request

    // if colliding with any entities,
    for (const auto& coll : physics.collision_stay) {

      const auto a = static_cast<entt::entity>(coll.ent_id_0);
      const auto b = static_cast<entt::entity>(coll.ent_id_1);

      const auto* a_pickupabble = r.try_get<AbleToBePickedUp>(a);
      const auto* b_pickupabble = r.try_get<AbleToBePickedUp>(b);

      if (a == entity && b_pickupabble) {
        dead.dead.emplace(b); // picked up item
        // todo: something interesting with item pickup
        // hack: just add xp counter for moment
        player_comp.picked_up_xp++;
      }
      if (b == entity && b_pickupabble) {
        dead.dead.emplace(a); // picked up item
        // todo: something interesting with item pickup
        // hack: just add xp counter for moment
        player_comp.picked_up_xp++;
      }
    }

    //
  }
}

} // namespace game2d
