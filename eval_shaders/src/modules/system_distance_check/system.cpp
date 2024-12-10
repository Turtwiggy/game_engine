#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"

namespace game2d {

void
update_distance_check_system(entt::registry& r)
{
  const auto& view = r.view<DistanceCheckComponent>();
  for (const auto& [e, distance_c] : view.each()) {

    // the distance check outlived the entities it was monitoring
    if (!r.valid(distance_c.e0) || !r.valid(distance_c.e1)) {
      r.destroy(e);
      continue;
    }

    const auto p0 = get_position(r, distance_c.e0);
    const auto p1 = get_position(r, distance_c.e1);
    const glm::vec2 dir = p1 - p0;
    const float d2 = dir.x * dir.x + dir.y * dir.y;

    if (distance_c.type == DISTANCE_CHECK_TYPE::LESS_THAN_OR_EQUAL) {
      if (d2 <= distance_c.d2) {
        distance_c.action(r, distance_c);

        // destroy the check...
        if (r.valid(e))
          r.destroy(e);
      }
    }

    else if (distance_c.type == DISTANCE_CHECK_TYPE::GREATER_THAN_OR_EQUAL) {
      if (d2 >= distance_c.d2) {
        distance_c.action(r, distance_c);

        // destroy the check...
        if (r.valid(e))
          r.destroy(e);
      }
    }
  }

  //
}

} // namespace game2d