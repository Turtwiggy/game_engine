#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/ui_economy/components.hpp"

namespace game2d {

void
update_resolve_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // some collisions result in dead entities
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // some collisions result in extra money
  auto& econ = get_first_component<SINGLETON_Economy>(r);

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

    // bullet-enemy collision
    {
      const auto& [actor_enemy, actor_bullet] =
        collision_of_interest(a, b, a_type, b_type, EntityType::actor_enemy, EntityType::actor_bullet);
      if (actor_enemy != entt::null && actor_bullet != entt::null) {
        dead.dead.emplace(actor_bullet); // kill bullet
        const auto& from = actor_bullet;
        const auto& to = actor_enemy;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }
    }
    // arrow-enemy collision
    {
      const auto& [actor_enemy, actor_arrow] =
        collision_of_interest(a, b, a_type, b_type, EntityType::actor_enemy, EntityType::actor_arrow);
      if (actor_enemy != entt::null && actor_arrow != entt::null) {
        dead.dead.emplace(actor_arrow); // kill arrow
        const auto& from = actor_arrow;
        const auto& to = actor_enemy;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }
    }

    // bullet-spawner collision
    {
      const auto& [actor_spawner, actor_bullet] =
        collision_of_interest(a, b, a_type, b_type, EntityType::actor_spawner, EntityType::actor_bullet);
      if (actor_spawner != entt::null && actor_bullet != entt::null) {
        dead.dead.emplace(actor_bullet); // kill bullet
        const auto& from = actor_bullet;
        const auto& to = actor_spawner;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }
    }
    // arrow-spawner collision
    {
      const auto& [actor_spawner, actor_arrow] =
        collision_of_interest(a, b, a_type, b_type, EntityType::actor_spawner, EntityType::actor_arrow);
      if (actor_spawner != entt::null && actor_arrow != entt::null) {
        dead.dead.emplace(actor_arrow); // kill arrow
        const auto& from = actor_arrow;
        const auto& to = actor_spawner;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }
    }

    // player-enemy collision
    {
      const auto& [actor_player, actor_enemy] =
        collision_of_interest(a, b, a_type, b_type, EntityType::actor_player, EntityType::actor_enemy);
      if (actor_player != entt::null && actor_enemy != entt::null) {
        {
          dead.dead.emplace(actor_enemy); // kill enemy
          const auto& from = actor_enemy;
          const auto& to = actor_player;
          r.emplace<DealDamageRequest>(r.create(), from, to);
        }
      }
    }

    // hearth-enemy collision
    {
      const auto& [actor_hearth, actor_enemy] =
        collision_of_interest(a, b, a_type, b_type, EntityType::actor_hearth, EntityType::actor_enemy);
      if (actor_hearth != entt::null && actor_enemy != entt::null) {
        dead.dead.emplace(actor_enemy); // kill enemy
        const auto& from = actor_enemy;
        const auto& to = actor_hearth;
        r.emplace<DealDamageRequest>(r.create(), from, to);
      }
    }
  }
}

} // namespace game2d