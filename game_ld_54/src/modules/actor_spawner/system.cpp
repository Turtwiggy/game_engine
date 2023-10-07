#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};
// const float safe_radius_around_player = 8000.0f;

void
update_spawner_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<const TransformComponent, SpawnerComponent, AttackCooldownComponent>();
  for (const auto& [entity, transform, spawner, cooldown] : view.each()) {

    if (cooldown.on_cooldown)
      return;

    if (spawner.continuous_spawn) {

      CreateEntityRequest req;
      req.type = spawner.type_to_spawn;
      req.transform = transform; // at current pos
      r.emplace<CreateEntityRequest>(r.create(), req);

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d