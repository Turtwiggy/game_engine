#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "physics/components.hpp"


#include "imgui.h"

#include <utility>

namespace game2d {

// What items should this drop on death?
//
const std::vector<std::pair<EntityType, int>> item_weight_map{
  // clang-format off
      {EntityType::empty_no_transform, 50}, 
      {EntityType::actor_pickup_xp, 30},
      {EntityType::actor_pickup_doubledamage, 20},
  // clang-format on
};

std::pair<float, EntityType>
generate_item()
{
  // sum the weights
  float total_weight = 0;
  for (const auto& [ent_type, ent_weight] : item_weight_map)
    total_weight += ent_weight;

  // TODO: BAD. FIX.
  // choose the item
  static engine::RandomState rnd(0);
  const float random = engine::rand_det_s(rnd.rng, 0.0f, total_weight);

  // Create the item
  int weight_acculum = 0;
  for (const auto& [k, w] : item_weight_map) {
    weight_acculum += w;
    if (weight_acculum >= random)
      return { random, k };
  }
  return { random, EntityType::empty_no_transform };
}

void
update_intent_drop_item_system(entt::registry& r)
{
  // "SINGLETON_EntityBinComponent" gets cleared every FixedUpdate().
  // If no FixedUpdate() occurs, this could spawn multiple items.
  // const auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // Also, if two FixedUpdate() occur back to back,
  // An entity could be killed and removed the next frame.

  // Temp solution:
  // The update_attack_cooldown_system() is in Update() not FixedUpdate()
  // even if two fixedupdate() occurs, the entity will take damage in Update(),
  // and this gets processed before the end of the next update().

  // Dead things drop xp items
  //

  // a frame behind
  static float last_generated_random = 0;

  //
  // Something died and wants to drop an item
  //
  const auto killable_view = r.view<HealthComponent, AbleToDropItem>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, e_hp, enemy_comp] : killable_view.each()) {
    if (e_hp.hp > 0)
      continue; // its still allive!

    r.remove<AbleToDropItem>(e); // dont drop items multiple times

    const auto [random, item_type] = generate_item();

    // debug the random number
    last_generated_random = random;

    // Enemy did not drop an item
    if (item_type == EntityType::empty_no_transform)
      continue;

    // create an item
    const auto item_e = r.create();
    r.emplace<EntityTypeComponent>(item_e, item_type);

    // create a new request
    // note: could drop multiple items if wanted
    WantsToDrop req;
    req.items.push_back(item_e);
    r.emplace_or_replace<WantsToDrop>(e, req);
  }

  //
  // Process WantsToDrop Requests
  //
  const auto& view = r.view<const AABB, WantsToDrop>(entt::exclude<WaitForInitComponent>);
  const auto& picked_up_items_view = r.view<PickedUpByComponent>(entt::exclude<WaitForInitComponent>);

  for (const auto& [entity, dead_aabb, request] : view.each()) {
    //
    // Check all items in the request
    // Create that item new
    //
    for (const auto& item : request.items) {
      const auto e = create_gameplay(r, r.get<EntityTypeComponent>(item).type);
      set_position(r, e, dead_aabb.center);
    }
    //
    // Check if this entity is holding any item
    // Stop this item being carried
    //
    for (const auto& [e, picked_up] : picked_up_items_view.each()) {
      if (picked_up.entity != entity)
        continue; // picked up by someone else
      //
      // you're picked up by picked_up.entity
      // and that parent entity wants to drop you
      //
      r.remove<PickedUpByComponent>(e);
      r.remove<FollowTargetComponent>(e);
      r.remove<HasTargetPositionComponent>(e);
      r.remove<SetVelocityToTargetComponent>(e);

      // hmm
      auto& vel = r.get<VelocityComponent>(e);
      vel.x = 0;
      vel.y = 0;
      vel.remainder_x = 0;
      vel.remainder_y = 0;

      r.emplace<AbleToBePickedUp>(e);
    }
  }
  r.remove<WantsToDrop>(view.begin(), view.end()); // requests done...
};

} // namespace game2d
