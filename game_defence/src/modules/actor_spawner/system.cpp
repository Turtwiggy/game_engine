#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

// each wave should have a "theme"
// e.g. flying, horse-back, etc
// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};

void
update_spawner_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  // const float dt = milliseconds_dt / 1000.0f;

  const auto& player_view = r.view<const PlayerComponent, const AABB>();

  const auto& view = r.view<const AABB, const TransformComponent, SpawnerComponent, AttackCooldownComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, aabb, transform, spawner, cooldown] : view.each()) {

    if (cooldown.on_cooldown)
      continue;

    // Check if any players are in range
    bool ok_to_spawn = true;
    if (const auto* player_constraint = r.try_get<OnlySpawnInRangeOfAnyPlayerComponent>(entity)) {
      for (const auto& [player_e, player_comp, player_aabb] : player_view.each()) {
        const glm::vec2 d = glm::vec2(player_aabb.center) - glm::vec2(transform.position.x, transform.position.y);
        const float current_distance2 = d.x * d.x + d.y * d.y;
        if (current_distance2 > player_constraint->distance2) {
          ok_to_spawn = false;
          break;
        }
      }
    }
    if (!ok_to_spawn)
      continue; // dont activate this spawner. No players in range.

    if (spawner.continuous_spawn) {

      // TODO: replace
      const auto& first_type = spawner.types_to_spawn[0];

      const auto e = create_gameplay(r, first_type);
      r.get<AABB>(e).center = aabb.center;
      r.get<TransformComponent>(e).position = transform.position;

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d