#include "resolve_collisions.hpp"

#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/cursor/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"

#include <format>

void
game2d::update_resolve_collisions_system(Game& game)
{
  auto& r = game.state;
  const auto& physics = game.physics;

  const auto clean = [&r](entt::entity& e) {
    // remove renderable
    auto* sc = r.try_get<SpriteComponent>(e);
    if (sc)
      r.remove<SpriteComponent>(e);
    auto* scc = r.try_get<SpriteColourComponent>(e);
    if (scc)
      r.remove<SpriteColourComponent>(e);
    auto* tc = r.try_get<TransformComponent>(e);
    if (tc)
      r.remove<TransformComponent>(e);
    // remove physics?
    if (auto* ptc = r.try_get<PhysicsTransformComponent>(e))
      r.remove<PhysicsTransformComponent>(e);
    if (auto* psc = r.try_get<PhysicsSolidComponent>(e))
      r.remove<PhysicsSolidComponent>(e);
    if (auto* pac = r.try_get<PhysicsActorComponent>(e))
      r.remove<PhysicsActorComponent>(e);
  };

  { // take damage on collision
    const auto& view = r.view<const WasCollidedWithComponent, TakeDamageComponent>();
    for (auto [entity, coll, damages] : view.each()) {
      const int collision_damage = 1;
      damages.damage.push_back(collision_damage);
    }
  }

  {
    // TODO: flash sprite on collision
    // const auto& view = r.view<WasCollidedWithComponent, HealthComponent>();
  }

  { // what's colliding with the cursor?

    std::vector<EntityType> valid_types{
      EntityType::enemy_orc,
      EntityType::enemy_troll,
    };

    const auto cursor_coll_behaviour = [&clean, &r, &valid_types](
                                         const uint32_t a, const uint32_t b, const entt::entity ent_a) {
      if (a == static_cast<uint32_t>(ent_a)) {
        auto other = static_cast<entt::entity>(b);
        auto tag = r.get<TagComponent>(other);
        auto type = r.get<EntityTypeComponent>(other);
        auto contains =
          std::find_if(valid_types.begin(), valid_types.end(), [&type](const EntityType& t) { return t == type.type; });
        if (contains != valid_types.end()) {
          // entity is colliding with something interesting!
          r.emplace_or_replace<CollidingWithCursorComponent>(other);
        }
      }
    };

    for (auto [cursor_entity, cursor] : r.view<FreeCursorComponent>().each()) {
      for (const auto& coll : physics.collision_stay) {
        cursor_coll_behaviour(coll.ent_id_0, coll.ent_id_1, cursor.backdrop);
        cursor_coll_behaviour(coll.ent_id_1, coll.ent_id_0, cursor.backdrop);
      }
    }
  }

  {
    // are we on an item we can pickup?

    std::vector<EntityType> valid_types{
      EntityType::potion,
      EntityType::scroll_damage_nearest,
      EntityType::scroll_damage_selected_on_grid,
    };

    const auto pickup_behaviour = [&clean, &r, &valid_types](
                                    const uint32_t a, const uint32_t b, const entt::entity ent_a) {
      if (a == static_cast<uint32_t>(ent_a)) {
        auto other = static_cast<entt::entity>(b);
        auto type = r.get<EntityTypeComponent>(other);
        auto contains =
          std::find_if(valid_types.begin(), valid_types.end(), [&type](const EntityType& t) { return t == type.type; });
        if (contains != valid_types.end()) {
          // entity is colliding with something interesting!

          auto& purchase = r.get_or_emplace<WantsToPurchase>(ent_a); // purchase from the floor?
          purchase.items.push_back(other);
          clean(other);
        }
      }
    };

    const auto& players = r.view<PlayerComponent>();
    for (auto [player_entity, player] : players.each()) {
      for (const auto& p : physics.collision_stay) {
        pickup_behaviour(p.ent_id_0, p.ent_id_1, player_entity);
        pickup_behaviour(p.ent_id_1, p.ent_id_0, player_entity);
      }
    }
  }

  // clear all WasCollidedWithComponents
  const auto& view = r.view<WasCollidedWithComponent>();
  r.remove<WasCollidedWithComponent>(view.begin(), view.end());
};