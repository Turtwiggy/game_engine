#include "modules/items_pickup/system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"

#include <iostream>
#include <optional>
#include <vector>

namespace game2d {

void
give_player_xp_item(entt::registry& r, const entt::entity& player_e, const entt::entity& item_e)
{
  if (const auto* able_to_be_picked_up = r.try_get<AbleToBePickedUp>(item_e)) {

    auto& player_c = r.get<PlayerComponent>(player_e);

    const auto item_type = r.get<EntityTypeComponent>(item_e).type;
    if (item_type == EntityType::actor_pickup_xp)
      player_c.picked_up_xp += 1;

    // Give the item to the player
    r.remove<AABB>(item_e);
    r.remove<TransformComponent>(item_e);
    r.emplace<HasParentComponent>(item_e, player_e);

    // dont pick up item multiple times
    r.remove<AbleToBePickedUp>(item_e);
  }
};

void
update_intent_pickup_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // System: check collisions between player and dropped items
  //
  for (const auto& coll : physics.collision_stay) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);
    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    const auto [a_ent, b_ent] = collision_of_interest<ItemComponent, PlayerComponent>(r, a, b);
    if (a_ent != entt::null && b_ent != entt::null) {
      const auto item = a_ent;
      const auto player = b_ent;
      give_player_xp_item(r, player, item);
    }
  }
}

} // namespace game2d
