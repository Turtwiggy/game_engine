#include "resolve_collisions.hpp"

#include "game/components/actors.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"

#include <format>

void
game2d::update_resolve_collisions_system(Game& game)
{
  auto& r = game.state;
  const auto& physics = game.physics;

  { // take damage on collision
    const auto& view = r.view<const WasCollidedWithComponent, TakeDamageComponent>();
    for (auto [entity, coll, damages] : view.each()) {
      const int collision_damage = 1;
      damages.damage.push_back(collision_damage);
    }
  }

  { // TODO: flash sprite on collision
    const auto& view = r.view<WasCollidedWithComponent, HealthComponent>();
  }

  {
    // are we on an item we can pickup?
    std::vector<EntityType> valid_types{
      EntityType::potion,
      EntityType::scroll_damage_nearest,
    };

    const auto& players = r.view<PlayerComponent>();
    for (auto [player_entity, player] : players.each()) {
      for (const auto& p : physics.collision_stay) {

        if (p.ent_id_0 == static_cast<uint32_t>(player_entity)) {
          auto other = static_cast<entt::entity>(p.ent_id_1);
          auto type = r.get<EntityTypeComponent>(other);

          auto contains = std::find_if(
            valid_types.begin(), valid_types.end(), [&type](const EntityType& t) { return t == type.type; });
          if (contains != valid_types.end()) {

            // PURCHASE FROM THE FLOOR?
            auto& purchase = r.get_or_emplace<WantsToPurchase>(player_entity);
            purchase.items.push_back(other);

            // remove renderable
            auto* sc = r.try_get<SpriteComponent>(other);
            if (sc)
              r.remove<SpriteComponent>(other);
            auto* scc = r.try_get<SpriteColourComponent>(other);
            if (scc)
              r.remove<SpriteColourComponent>(other);
            auto* tc = r.try_get<TransformComponent>(other);
            if (tc)
              r.remove<TransformComponent>(other);
          }
        }

        if (p.ent_id_1 == static_cast<uint32_t>(player_entity)) {
          auto other = static_cast<entt::entity>(p.ent_id_0);
          auto type = r.get<EntityTypeComponent>(other);

          auto contains = std::find_if(
            valid_types.begin(), valid_types.end(), [&type](const EntityType& t) { return t == type.type; });
          if (contains != valid_types.end()) {

            // PURCHASE FROM THE FLOOR?
            auto& purchase = r.get_or_emplace<WantsToPurchase>(player_entity);
            purchase.items.push_back(other);

            // remove renderable
            auto* sc = r.try_get<SpriteComponent>(other);
            if (sc)
              r.remove<SpriteComponent>(other);
            auto* scc = r.try_get<SpriteColourComponent>(other);
            if (scc)
              r.remove<SpriteColourComponent>(other);
            auto* tc = r.try_get<TransformComponent>(other);
            if (tc)
              r.remove<TransformComponent>(other);
          }
        }
      }
    }
  }

  // clear all waWasCollidedWithComponents
  const auto& view = r.view<WasCollidedWithComponent>();
  r.remove<WasCollidedWithComponent>(view.begin(), view.end());
};