#include "system.hpp"

#include "components.hpp"
#include "modules/actors/helpers.hpp"

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

    const auto& p0 = get_position(r, distance_c.e0);
    const auto& p1 = get_position(r, distance_c.e1);
    const auto dir = p1 - p0;
    const int d2 = dir.x * dir.x + dir.y * dir.y;
    if (d2 <= distance_c.d2) {
      distance_c.action(r);

      // destroy the actual check...
      // this doesn't have to be this way...
      // but just trial out this system
      r.destroy(e);
    }
  }

  //
}

} // namespace game2d