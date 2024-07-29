#include "system.hpp"

#include "entt/helpers.hpp"
#include "helpers/entity_pool.hpp"
#include "helpers/line.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/debug_pathfinding/components.hpp"
#include "modules/grid/components.hpp"

namespace game2d {

void
update_debug_pathfinding_system(entt::registry& r)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  const auto& view = r.view<GeneratedPathComponent>();
  int desired_lines = 0;
  for (const auto& [e, path] : view.each())
    desired_lines += (path.path.size() - 1); // -1, beacuse 1 less line than points

  const auto debug_e = get_first<SINGLE_DebugPathLines>(r);
  if (debug_e == entt::null)
    destroy_first_and_create<SINGLE_DebugPathLines>(r);
  auto& debug_path_lines = get_first_component<SINGLE_DebugPathLines>(r);
  debug_path_lines.pool.update(r, desired_lines);

  int line_idx = 0;
  for (const auto& [e, path_c] : view.each()) {
    const auto& path = path_c.path;
    if (path.size() == 0)
      continue;
    for (int i = 0; i < path.size() - 1; i++) {
      const auto offset = glm::ivec2{ map.tilesize / 2, map.tilesize / 2 };
      const auto cur = (glm::ivec2{ path[i].x, path[i].y } * map.tilesize) + offset;
      const auto nxt = (glm::ivec2{ path[i + 1].x, path[i + 1].y } * map.tilesize) + offset;

      if (line_idx >= desired_lines)
        continue; // why dis happening?

      const auto debug_e = debug_path_lines.pool.instances[line_idx];
      set_transform_with_line(r, debug_e, generate_line(cur, nxt, 2));

      line_idx++;
    }
  }
};

} // namespace game2d