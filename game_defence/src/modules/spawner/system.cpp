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

    spawner.time_between_spawns_left -= dt;

    if (spawner.time_between_spawns_left < 0.0f) {

      // create spawn request
      CreateEntityRequest req;
      req.entity_type = spawner.type_to_spawn;
      req.position = transform.position;
      auto e = r.create();
      r.emplace<CreateEntityRequest>(e, req);

      // reset timer
      spawner.time_between_spawns_left = time_between_spawns;
    }
  }
}

} // namespace game2d