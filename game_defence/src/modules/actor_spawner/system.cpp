#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};

void
update_spawner_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<const AABB, const TransformComponent, SpawnerComponent, AttackCooldownComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, aabb, transform, spawner, cooldown] : view.each()) {

    if (cooldown.on_cooldown)
      return;

    if (spawner.continuous_spawn) {

      const auto e = create_gameplay(r, spawner.type_to_spawn);
      r.get<AABB>(e).center = aabb.center;
      r.get<TransformComponent>(e).position = transform.position;

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d