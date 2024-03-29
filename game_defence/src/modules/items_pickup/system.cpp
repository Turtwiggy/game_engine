#include "modules/items_pickup/system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_group/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_powerup_doubledamage/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"


#include <iostream>
#include <optional>
#include <vector>

namespace game2d {

void
give_player_doubledamage_item(entt::registry& r, const entt::entity& player_e, const entt::entity& item_e)
{
  PowerupDoubleDamage default_powerup;

  // player already had a doubledamage powerup... extend it?
  if (auto* powered_up = r.try_get<PowerupDoubleDamage>(player_e))
    powered_up->timeleft += default_powerup.timeleft;
  else
    r.emplace<PowerupDoubleDamage>(player_e, default_powerup);
}

void
resolve_player_item_collision(entt::registry& r, const entt::entity& a, const entt::entity& b)
{
  if (a == entt::null)
    return;
  if (b == entt::null)
    return;
  const auto& item = a;
  const auto& player = b;

  if (const auto* able_to_be_picked_up = r.try_get<AbleToBePickedUp>(item)) {
    // The player collided with an item. What type of item is it?
    const auto& entity_type = r.get<EntityTypeComponent>(item).type;

    if (entity_type == EntityType::actor_pickup_doubledamage)
      give_player_doubledamage_item(r, player, item);

    // Give the item to the player
    r.remove<AABB>(item);
    r.remove<TransformComponent>(item);
    r.emplace<HasParentComponent>(item, player);

    // dont pick up item multiple times
    r.remove<AbleToBePickedUp>(item);
  }
};

void
resolve_player_actor_collision(entt::registry& r, const entt::entity& a, const entt::entity& b)
{
  if (a == entt::null)
    return;
  if (b == entt::null)
    return;
  const auto& player = a;
  const auto& actor = b;

  if (auto* player_wants_to_pickup = r.try_get<WantsToPickUp>(player)) {
    std::cout << "processing pickup request...!\n";

    // make the actor follow the player if it's picked up
    //
    if (auto* actor_able_to_be_pickedup = r.try_get<AbleToBePickedUp>(actor)) {
      r.emplace_or_replace<PickedUpByComponent>(actor, player);
      r.emplace_or_replace<FollowTargetComponent>(actor, player);
      r.emplace_or_replace<HasTargetPositionComponent>(actor);
      r.emplace_or_replace<SetVelocityToTargetComponent>(actor);

      r.remove<AbleToBePickedUp>(actor); // request processed...
    }

    r.remove<WantsToPickUp>(player); // request processed...
  }
}

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
    resolve_player_item_collision(r, a_ent, b_ent);

    const auto [a_player, b_group] = collision_of_interest<PlayerComponent, GroupComponent>(r, a, b);
    resolve_player_actor_collision(r, a_player, b_group);
  }
}

} // namespace game2d
