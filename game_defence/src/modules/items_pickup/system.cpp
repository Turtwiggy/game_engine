#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"

#include <iostream>

namespace game2d {

void
update_intent_pickup_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  for (const auto& [player_entity, player, request] : r.view<PlayerComponent, WantsToPickUp>().each()) {

    const auto& zone = player.pickup_area;

    // if colliding with any entities,
    for (const auto& coll : physics.collision_stay) {
      if (coll.ent_id_0 == static_cast<uint32_t>(zone)) {
        const auto other = static_cast<entt::entity>(coll.ent_id_1);
        if (const auto* pickupabble = r.try_get<AbleToBePickedUp>(other)) {
          // pick it up!
          dead.dead.emplace(other);
          player.picked_up_xp++; // hack
        }
      }
      if (coll.ent_id_1 == static_cast<uint32_t>(zone)) {
        // something is colliding with the pickup zone
        const auto other = static_cast<entt::entity>(coll.ent_id_0);
        if (const auto* pickupabble = r.try_get<AbleToBePickedUp>(other)) {
          // pick it up!
          dead.dead.emplace(other);
          player.picked_up_xp++; // hack
        }
      }
    }

    r.remove<WantsToPickUp>(player_entity);
  }

  // update pickup zone positions for collisions next frmae
  for (const auto& [entity, player, player_aabb] : r.view<const PlayerComponent, const AABB>().each()) {
    const auto& zone = player.pickup_area;
    auto& zone_aabb = r.get<AABB>(zone);
    zone_aabb.center = player_aabb.center;
  }
}

} // namespace game2d
