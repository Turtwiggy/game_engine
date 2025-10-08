#include "pch.hpp"

#include "island_return_to_boat_land_immunity_system.hpp"

#include "modules/systems/system_island_return_to_boat_land_immunity/island_return_to_boat_land_immunity_components.hpp"

namespace game2d {

void
update_island_return_to_boat_land_immunity_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto view = r.view<IslandReturnToBoatLandImmunity>();

  for (const auto& [e, immunity_c] : view.each()) {
    immunity_c.immunity_time_Left -= dt;

    if (immunity_c.immunity_time_Left <= 0.0f)
      r.remove<IslandReturnToBoatLandImmunity>(e);
  }
}

} // namespace game2d