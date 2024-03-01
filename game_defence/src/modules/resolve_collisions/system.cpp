#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/resolve_collisions/helpers.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // some collisions result in dead entities
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  for (const auto& coll : physics.collision_enter) {

    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    const auto* a_atk = r.try_get<AttackComponent>(a);
    const auto* a_def = r.try_get<HealthComponent>(a);
    const auto* a_team = r.try_get<TeamComponent>(a);
    const auto* b_atk = r.try_get<AttackComponent>(b);
    const auto* b_def = r.try_get<HealthComponent>(b);
    const auto* b_team = r.try_get<TeamComponent>(b);

    // Dealing Damage
    //
    // deal damage to b
    if (a_atk && b_def && a_team && b_team) {
      if (a_team->team == b_team->team)
        continue; // no team damage
      dead.dead.emplace(a);
      const entt::entity from = a;
      const entt::entity to = b;
      r.emplace<DealDamageRequest>(r.create(), from, to);
    }
    // deal damage to a
    if (b_atk && a_def && a_team && b_team) {
      if (a_team->team == b_team->team)
        continue; // no team damage
      dead.dead.emplace(b);
      const entt::entity from = b;
      const entt::entity to = a;
      r.emplace<DealDamageRequest>(r.create(), from, to);
    }
  }

  for (const auto& coll : physics.collision_stay) {

    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    // check if an enemy is colling with player line of sight
    //
    const auto [a_ent, b_ent] = collision_of_interest<const LineOfSightComponent, const EnemyComponent>(r, a, b);
    if (a_ent != entt::null && b_ent != entt::null) {
      // an enemy is colliding with line of sight!
      const auto& parent = r.get<HasParentComponent>(a_ent); // type player
      r.emplace_or_replace<WantsToShoot>(parent.parent);
    }
  }
}

} // namespace game2d