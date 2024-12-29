#include "unit_enter_blackhole_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/map/components.hpp"
#include "unit_enter_blackhole_components.hpp"

namespace game2d {

void
handle_unit_enter_blackhole(entt::registry& r, const GridPositionChangedEvent& evt)
{
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

  if (evt.e == entt::null)
    return;

  //
  // immediately kill the unit
  // if it's in a blackhole
  //
  const auto& map_es = map_c.map[evt.dst_idx];
  for (const auto map_e : map_es) {
    auto* kill_c = r.try_get<OnCollisionKill>(map_e);
    if (kill_c == nullptr)
      continue;
    dead.dead.emplace(evt.e);
  }
}

} // namespace game2d