#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // some collisions result in dead entities
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // const auto& collision_of_interest = [](const entt::entity& a_ent,
  //                                        const entt::entity& b_ent,
  //                                        const EntityType& a,
  //                                        const EntityType& b,
  //                                        const EntityType& a_actual,
  //                                        const EntityType& b_actual) -> std::pair<entt::entity, entt::entity> {
  //   if (a == a_actual && b == b_actual)
  //     return { a_ent, b_ent };
  //   if (a == b_actual && b == a_actual)
  //     return { b_ent, a_ent };
  //   return { entt::null, entt::null };
  // };

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
}

} // namespace game2d