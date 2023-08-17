#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "lifecycle/components.hpp"
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

  const auto& view = r.view<const TransformComponent, SpawnerComponent>();
  for (auto [entity, transform, spawner] : view.each()) {

    if (spawner.continuous_spawn)
      spawner.time_between_spawns_left -= dt;

    if (spawner.continuous_spawn && spawner.time_between_spawns_left < 0.0f) {

      CreateEntityRequest req;
      req.type = spawner.type_to_spawn;
      // spawn at current position
      req.position = transform.position;
      r.emplace<CreateEntityRequest>(r.create(), req);

      // reset timer
      spawner.time_between_spawns_left = time_between_spawns;
    }
  }
}

} // namespace game2d