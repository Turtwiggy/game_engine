#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/physics/components.hpp"

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // some collisions result in dead entities
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& collision_of_interest = [](const entt::entity& a_ent,
                                         const entt::entity& b_ent,
                                         const EntityType& a,
                                         const EntityType& b,
                                         const EntityType& a_actual,
                                         const EntityType& b_actual) -> std::pair<entt::entity, entt::entity> {
    if (a == a_actual && b == b_actual)
      return { a_ent, b_ent };
    if (a == b_actual && b == a_actual)
      return { b_ent, a_ent };
    return { entt::null, entt::null };
  };

  for (const auto& coll : physics.collision_enter) {

    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    if (!r.valid(a) || !r.valid(b))
      continue;

    const auto& a_type = r.get<EntityTypeComponent>(a).type;
    const auto& b_type = r.get<EntityTypeComponent>(b).type;

    {
      auto* a_atk = r.try_get<AttackComponent>(a);
      auto* a_def = r.try_get<HealthComponent>(a);
      auto* b_atk = r.try_get<AttackComponent>(b);
      auto* b_def = r.try_get<HealthComponent>(b);

      // deal damage to b
      if (a_atk && b_def) {
        const entt::entity from = a;
        const entt::entity to = b;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }

      // deal damage to a
      if (b_atk && a_def) {
        const entt::entity from = b;
        const entt::entity to = a;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }
    }

    // todo: dont just *kill* enemies immediately on collision
    // should do something more interesting give the enemy damage
    // from the attack but then dont take any more damage from it

    std::vector<EntityType> types_to_destroy{
      EntityType::bullet_bow,
      EntityType::bullet_default,
      EntityType::enemy_grunt,
      EntityType::enemy_shotgunner,
      EntityType::enemy_sniper //
    };
    bool a_is_of_type = std::find(types_to_destroy.begin(), types_to_destroy.end(), a_type) != types_to_destroy.end();
    if (a_is_of_type)
      dead.dead.emplace(a);
    bool b_is_of_type = std::find(types_to_destroy.begin(), types_to_destroy.end(), b_type) != types_to_destroy.end();
    if (b_is_of_type)
      dead.dead.emplace(b);
  }
}

} // namespace game2d