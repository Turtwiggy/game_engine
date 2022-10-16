#include "helpers.hpp"

#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "modules/cursor/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace game2d {

void
clean(Game& game, entt::entity& e)
{
  auto& r = game.state;
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
}

std::optional<entt::entity>
contains(entt::registry& r,
         const uint32_t& a,
         const uint32_t& b,
         const entt::entity& check,
         const std::vector<EntityType>& valid)
{
  if (a == static_cast<uint32_t>(check)) {
    auto other = static_cast<entt::entity>(b);
    auto type = r.get<EntityTypeComponent>(other);
    auto contains = std::find_if(valid.begin(), valid.end(), [&type](const EntityType& t) { return t == type.type; });
    if (contains != valid.end())
      return other;
  }
  return std::nullopt;
};

// returns [check, other]
std::tuple<entt::entity, entt::entity>
collision_of_interest(entt::registry& r,
                      const Collision2D& coll,
                      const entt::entity& check,
                      const std::vector<EntityType>& valid)
{
  auto a = contains(r, coll.ent_id_0, coll.ent_id_1, check, valid);
  if (a != std::nullopt)
    return { static_cast<entt::entity>(coll.ent_id_0), static_cast<entt::entity>(coll.ent_id_1) };

  auto b = contains(r, coll.ent_id_1, coll.ent_id_0, check, valid);
  if (b != std::nullopt)
    return { static_cast<entt::entity>(coll.ent_id_1), static_cast<entt::entity>(coll.ent_id_0) };

  return { entt::null, entt::null };
};

// Want to know if a damageable object was collided with
void
check_if_damageable_received_collision(Game& game)
{
  auto& r = game.state;
  const auto& view = r.view<const WasCollidedWithComponent, TakeDamageComponent>();
  for (auto [entity, coll, damages] : view.each()) {
    // FIX: damage actually given here! (seems wrong)
    const int collision_damage = 1;
    damages.damage.push_back(collision_damage);
  }
};

// Want to know if an enemy collided with the cursor
void
check_if_collided_with_cursor(Game& game)
{
  auto& r = game.state;
  auto& physics = game.physics;

  // Note: this code below seems wrong.
  // .. the constraints
  std::vector<EntityType> valid_types{
    EntityType::enemy_orc,
    EntityType::enemy_troll,
  };

  // .. the check
  for (auto [cursor_entity, cursor] : r.view<FreeCursorComponent>().each()) {
    for (const auto& coll : physics.collision_stay) {
      auto [cursor_backdrop, other] = collision_of_interest(r, coll, cursor.backdrop, valid_types);
      if (other != entt::null)
        r.emplace_or_replace<CollidingWithCursorComponent>(other);
    }
  }
};

// Want to know if a player collided with a pickup
void
check_if_collided_with_pickup(Game& game)
{
  auto& r = game.state;
  auto& physics = game.physics;

  // .. the constraints
  std::vector<EntityType> valid_types{
    EntityType::potion,
    EntityType::scroll_damage_nearest,
    EntityType::scroll_damage_selected_on_grid,
  };

  // .. the check
  const auto& players = r.view<PlayerComponent>();
  for (auto [player_entity, player] : players.each()) {
    for (const auto& coll : physics.collision_stay) {
      auto [player_coll_ent, other] = collision_of_interest(r, coll, player_entity, valid_types);
      if (other != entt::null) {
        // purchase from the floor?
        auto& purchase = r.get_or_emplace<WantsToPurchase>(player_entity);
        purchase.items.push_back(other);
        clean(game, other);
      }
    }
  }
};

// Want to know if a player collided with the exit
void
check_if_collided_with_exit(Game& game)
{
  auto& r = game.state;
  auto& physics = game.physics;

  // .. the constraints
  std::vector<EntityType> valid_types{
    EntityType::exit,
  };

  const auto& players = r.view<PlayerComponent>();
  for (auto [player_entity, player] : players.each()) {
    for (const auto& coll : physics.collision_stay) {
      auto [player_coll_ent, other] = collision_of_interest(r, coll, player_entity, valid_types);
      if (player_coll_ent != entt::null) {
        CollidingWithExitComponent c;
        c.exit = other;
        r.emplace_or_replace<CollidingWithExitComponent>(player_coll_ent, c);
      }
    }
  }
};

} // namespace game2d