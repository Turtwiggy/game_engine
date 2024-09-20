#include "system.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_change_gun_z_index/helpers.hpp"

namespace game2d {

void
update_debug_map_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  const auto map_debug_e = get_first<DebugMapComponent>(r);
  if (map_debug_e == entt::null)
    create_empty<DebugMapComponent>(r);
  auto& map_debug = get_first_component<DebugMapComponent>(r);

  auto& pool = map_debug.pool;

  size_t amount = 0;
  for (size_t i = 0; i < map.map.size(); i++)
    amount += map.map[i].size();
  pool.update(r, amount);

  // debug all grid cells that contain something
  for (size_t i = 0; i < map.map.size(); i++) {
    const auto instance_e = pool.instances[i];

    for (const auto e : map.map[i]) {
      auto wp = engine::grid::index_to_world_position_center((int)i, map.xmax, map.ymax, map.tilesize);
      set_position(r, instance_e, wp);
      set_size(r, instance_e, { 8, 8 });
      set_colour(r, instance_e, { 0.75f, 0.25f, 0.25f, 1.0f });
      // set_z_index(r, instance_e, ZLayer::FOREGROUND);
    }
  }
}

} // namespace game2d