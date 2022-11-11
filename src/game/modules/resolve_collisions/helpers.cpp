#include "helpers.hpp"

#include "game/components/actors.hpp"
#include "game/components/events.hpp"
#include "game/helpers/check_equipment.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/items/components.hpp"
#include "game/modules/player/components.hpp"
#include "game/modules/rpg_xp/components.hpp"
#include "modules/cursor/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"

#include "magic_enum.hpp"
#include <glm/glm.hpp>

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
// This does the check based on valid EntityTypes (rather than entt components)
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

    int base_damage = 0;
    int extra_damage = 0;
    int mitigated_damage = 0;

    const auto& attacker = coll.instigator;
    const auto& defender = entity;

    // Only take damage if player involved
    // i.e. prevent enemy-enemy damages
    auto* player_was_attacker = r.try_get<PlayerComponent>(attacker);
    auto* player_was_defender = r.try_get<PlayerComponent>(defender);
    if (!(player_was_attacker || player_was_defender))
      continue;

    // Check everyone's stats?

    // check attacker str
    {
      const auto* attacker_stats = r.try_get<StatsComponent>(attacker);
      if (attacker_stats) {
        const auto& attacker_str = attacker_stats->str_level;
        base_damage += attacker_str;
      }
    }

    // check defender agi
    {
      const auto* defender_stats = r.try_get<StatsComponent>(defender);
      if (defender_stats) {
        const auto& defender_agi = defender_stats->agi_level;
        mitigated_damage += defender_agi;
      }
    }

    // Check everyone's equipment?

    // check all equipment slots
    for (int i = 0; i < magic_enum::enum_count<EquipmentSlot>(); i++) {
      const auto slot = magic_enum::enum_value<EquipmentSlot>(i);

      // If the attacker had a weapon, deal extra damage.
      {
        const auto potential_weapon = has_equipped(game, attacker, slot);
        if (potential_weapon != entt::null) {
          const auto* attack = r.try_get<AttackComponent>(potential_weapon);
          if (attack) {
            // BUG: dont use min damage.
            extra_damage += attack->min_damage;
          }
        }
      }

      // If the defender had a shield, mitigate some damage.
      {
        const auto potential_shield = has_equipped(game, defender, slot);
        if (potential_shield != entt::null) {
          const auto* defense = r.try_get<DefenseComponent>(potential_shield);
          if (defense) {
            // TODO: improve defense.
            mitigated_damage += defense->ac;
          }
        }
      }
    }

    RecieveDamageRequest dmg;
    dmg.base_damage = base_damage;
    dmg.extra_damage = extra_damage;
    dmg.mitigated_damage = mitigated_damage;

    damages.damage.push_back(dmg);
  }
};

// Want to know if an enemy collided with the cursor
void
check_if_enemy_collided_with_cursor(Game& game)
{
  auto& r = game.state;
  auto& physics = game.physics;

  // Note: this code below seems wrong.
  // .. the constraints
  std::vector<EntityType> valid_types{
    EntityType::actor_bat,
    EntityType::actor_troll,
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
check_if_player_collided_with_pickup(Game& game)
{
  auto& r = game.state;
  auto& physics = game.physics;

  // .. the check
  const auto& players = r.view<PlayerComponent>();
  for (auto [player_entity, player] : players.each()) {
    for (const auto& coll : physics.collision_stay) {
      auto a = static_cast<entt::entity>(coll.ent_id_0);
      auto b = static_cast<entt::entity>(coll.ent_id_1);
      auto* pickup_a = r.try_get<AbleToBePickedUp>(a);
      auto* pickup_b = r.try_get<AbleToBePickedUp>(b);
      if (a == player_entity && pickup_b) {
        // purchase from the floor?
        auto& purchase = r.get_or_emplace<WantsToPurchase>(player_entity);
        purchase.items.push_back(b);
        clean(game, b);
      }
      if (b == player_entity && pickup_a) {
        // purchase from the floor?
        auto& purchase = r.get_or_emplace<WantsToPurchase>(player_entity);
        purchase.items.push_back(a);
        clean(game, a);
      }
    }
  }
};

// Want to know if a player collided with the exit
void
check_if_player_collided_with_exit(Game& game)
{
  auto& r = game.state;
  auto& physics = game.physics;

  // .. the constraints
  std::vector<EntityType> valid_types{
    EntityType::tile_type_exit,
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