#include "system.hpp"

#include "maths/maths.hpp"
#include "modules/actor_dropoff_zone/components.hpp"

namespace game2d {

// this shouldnt be here
static engine::RandomState rnd;

void
update_actor_dropoffzone_request_items(entt::registry& r, uint64_t ms_dt)
{
  const float dt = ms_dt / 1000.0f;

  const auto& view = r.view<DropoffZoneComponent>();
  for (const auto& [entity, zone] : view.each()) {

    zone.timer -= dt;
    if (zone.timer > 0)
      continue; // not interested

    // Reset timer
    const float random_time = engine::rand_det_s(rnd.rng, zone.min_seconds_between_items, zone.max_seconds_between_items);
    zone.timer = random_time;

    // Add a request to the dropzone
    const float random_idx_f = engine::rand_det_s(rnd.rng, 0, zone.valid_items_to_request.size());
    const int random_idx = static_cast<int>(random_idx_f);
    const int random_item = zone.valid_items_to_request[random_idx];
    zone.requested_items.push_back(random_item);
  }
}

} // namespace game2d