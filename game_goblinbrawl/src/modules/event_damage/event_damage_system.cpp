#include "event_damage_system.hpp"

#include "engine/entt/helpers.hpp"
#include "event_damage_helpers.hpp"
#include "modules/event_damage/event_damage_components.hpp"

#include <algorithm>

namespace game2d {

void
update_event_damage_system(entt::registry& r, const float dt)
{
  auto queue_e = get_first<SINGLE_DamageQueue>(r);
  if (queue_e == entt::null)
    return;
  auto& queue_c = get_first_component<SINGLE_DamageQueue>(r);
  auto& q = queue_c.queue;

  // act on damage events that are no longer parry-able
  for (auto it = q.begin(); it != q.end();) {
    const auto [evt, timer] = (*it);
    if (timer >= queue_c.max_parry_time_s) {
      it = q.erase(it);

      if (r.valid(evt.from) && r.valid(evt.to))
        handle_damage_event_take_damage(r, evt);
    } else
      ++it;
  }

  // increment timer on all damage events
  std::for_each(q.begin(), q.end(), [&dt](auto& pair) { pair.second += dt; });
}

} // namespace game2d