#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "lifecycle/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_attack_cooldown/helpers.hpp"
#include "renderer/components.hpp"

namespace game2d {

// static std::map<int, float> wave_toughness{};
// static std::map<int, float> wave_damage{};
// static std::map<int, float> wave_speed{};
// const float safe_radius_around_player = 8000.0f;

// should be in something
static const float time_between_spawns = 2.0f;
static float time_between_spawns_left = time_between_spawns;

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
      // spawn at current position
      req.position = transform.position;
      r.emplace<CreateEntityRequest>(r.create(), req);

      reset_cooldown(cooldown);
    }
  }
}

} // namespace game2d