#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"

namespace game2d {

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
  const auto killable_view = r.view<HealthComponent, AbleToDropItem>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, e_hp, enemy_comp] : killable_view.each()) {
    if (e_hp.hp <= 0) {
      r.remove<AbleToDropItem>(e); // dont drop items multiple times

      // create an xp item
      const auto item_e = r.create();
      r.emplace<EntityTypeComponent>(item_e, EntityType::actor_pickup_xp);

      // create a new request
      WantsToDrop req;
      req.items.push_back(item_e);
      r.emplace_or_replace<WantsToDrop>(e, req);
    }
  }

  // WantsToDrop Requests
  //
  const auto& view = r.view<const AABB, WantsToDrop>(entt::exclude<WaitForInitComponent>);
  if (view.size_hint() > 0)
    std::cout << view.size_hint() << " entities want to drop items" << std::endl;

  for (const auto& [entity, dead_aabb, request] : view.each()) {
    for (const auto& item : request.items) {

      const auto e = create_gameplay(r, r.get<EntityTypeComponent>(item).type);

      // set aabb
      if (auto* aabb = r.try_get<AABB>(e))
        aabb->center = dead_aabb.center;

      // set transform
      r.get<TransformComponent>(e).position = glm::ivec3(dead_aabb.center.x, dead_aabb.center.y, 0);
    }
  }

  r.remove<WantsToDrop>(view.begin(), view.end()); // requests done...
};

} // namespace game2d
