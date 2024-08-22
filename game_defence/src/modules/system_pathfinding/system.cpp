#include "system.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"

#include "fmt/core.h"

#include <algorithm>

namespace game2d {

void
update_pathfinding_system(entt::registry& r, const float& dt)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map = get_first_component<MapComponent>(r);
  const float dt_ms = dt * 1000.0f;

  // if something was killed, remove it from the map
  const auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
  for (const auto& e : dead.dead) {

    // remove it from the map
    const auto& pred = [&e](const entt::entity& other) -> bool { return e == other; };
    auto it = std::find_if(map.map.begin(), map.map.end(), pred);
    if (it != map.map.end()) {
      uint64_t index = it - map.map.begin();
      if (map.map[index] == entt::null)
        fmt::println("Map: dead entity was not in map?");
      map.map[index] = entt::null;
    }
  }

  // anything with a path attached
  const auto& pathfinding = r.view<GeneratedPathComponent, StaticTargetComponent>();

  for (const auto& [e, path, target_c] : pathfinding.each()) {
    const auto cur = get_position(r, e);
    auto cur_gridpos = engine::grid::worldspace_to_grid_space(glm::vec2(cur.x, cur.y), map.tilesize);
    cur_gridpos.x = glm::clamp(cur_gridpos.x, 0, map.xmax - 1);
    cur_gridpos.y = glm::clamp(cur_gridpos.y, 0, map.ymax - 1);

    // get target grid position at the time which the pathfinding was calculated
    const auto dst_static = path.dst_pos;
    glm::ivec2 dst_gridpos_static = engine::grid::worldspace_to_grid_space(dst_static, map.tilesize);
    dst_gridpos_static.x = glm::clamp(dst_gridpos_static.x, 0, map.xmax - 1);
    dst_gridpos_static.y = glm::clamp(dst_gridpos_static.y, 0, map.ymax - 1);

    // try to get live target grid position
    // if no dst_ent is set, assume a static position
    std::optional<glm::ivec2> dst_gridpos_dynamic = std::nullopt;
    std::optional<glm::ivec2> dst_worldpos_dynamic = std::nullopt;
    if (path.dst_ent != entt::null) {
      dst_worldpos_dynamic = get_position(r, path.dst_ent);
      dst_gridpos_dynamic = engine::grid::worldspace_to_grid_space(dst_worldpos_dynamic.value(), map.tilesize);
      dst_gridpos_dynamic.value().x = glm::clamp(dst_gridpos_dynamic.value().x, 0, map.xmax - 1);
      dst_gridpos_dynamic.value().y = glm::clamp(dst_gridpos_dynamic.value().y, 0, map.ymax - 1);
    }

    // if (path.aim_for_exact_position) {
    //   dst_worldpos_dynamic = path.dst_pos;
    //   dst_gridpos_dynamic = engine::grid::worldspace_to_grid_space(dst_worldpos_dynamic.value(), map.tilesize);
    //   dst_gridpos_dynamic.value().x = glm::clamp(dst_gridpos_dynamic.value().x, 0, map.xmax - 1);
    //   dst_gridpos_dynamic.value().y = glm::clamp(dst_gridpos_dynamic.value().y, 0, map.ymax - 1);
    //   ImGui::Text("dst_gridpos: %i %i", dst_gridpos_dynamic.value().x, dst_gridpos_dynamic.value().y);
    // }

    // if (dst_gridpos_dynamic.has_value() && dst_gridpos_static != dst_gridpos_dynamic.value())
    //   ImGui::Text("Target has moved outside pathfinding??");

    // probably clicked either on invalid spot, or spot in same gridspace
    const bool in_same_gridspace = cur_gridpos == dst_gridpos_dynamic;
    if ((path.path.size() == 0 || in_same_gridspace) && dst_worldpos_dynamic.has_value()) {
      target_c.target = dst_worldpos_dynamic.value();
    }

    for (int i = 0; const auto p : path.path) {
      if (in_same_gridspace)
        continue; // probably shouldnt be in this for loop

      // not on our section in the path
      if (p != cur_gridpos) {
        i++;
        continue;
      }

      if (path.required_to_clear_path) {

        // move towards center of the current tile
        if (!path.path_cleared[i]) {
          const auto target_pos = engine::grid::grid_space_to_world_space(cur_gridpos, map.tilesize);
          target_c.target = target_pos;
          target_c.target.value() += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);
        }

        // check if within distance of the center of the gridpos
        if (!path.path_cleared[i]) {
          const auto d = get_position(r, e) - target_c.target.value();
          const float d2 = d.x * d.x + d.y * d.y;
          const float threshold = 6;
          if (d2 <= threshold) {
            path.path_cleared[i] = true;

            if (auto* lerping_to_target = r.try_get<LerpingToTarget>(e))
              r.remove<LerpingToTarget>(e);
          }
        }

        // need to have cleared the path for the next conditions
        if (!path.path_cleared[i])
          break;

        // If we've cleared the current gridtile path,
        // wait for a little bit to simulate "walking" steps
        if (path.wait_at_destination && path.wait_time_ms_left > 0) {
          path.wait_time_ms_left -= dt_ms;
          break;
        }

        // reset the wait time for the next path
        path.wait_time_ms_left = path.wait_time_ms;

        // aim for the next gridtile path
        const int next_idx = i + 1;
        const bool is_valid_next_index = next_idx <= static_cast<int>(path.path.size()) - 1;
        if (is_valid_next_index) {
          const auto next_pos = path.path[next_idx];
          const auto target_pos = engine::grid::grid_space_to_world_space(next_pos, map.tilesize);
          // center, not top left
          target_c.target = target_pos;
          target_c.target.value() += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);

          auto* lerping_to_target = r.try_get<LerpingToTarget>(e);
          if (lerping_to_target == nullptr) {
            LerpingToTarget data;
            data.a = cur;
            data.b = target_c.target.value();
            data.t = 0.0f;
            r.emplace_or_replace<LerpingToTarget>(e, data);
          }

          break;
        }

      } else {
        // aim for the next gridtile
        const int next_idx = i + 1;
        const bool is_valid_next_index = next_idx <= static_cast<int>(path.path.size()) - 1;
        if (is_valid_next_index) {
          const auto next_pos = path.path[next_idx];
          const auto target_pos = engine::grid::grid_space_to_world_space(next_pos, map.tilesize);
          target_c.target = target_pos; // center, not top left
          target_c.target.value() += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);
          break;
        }
      }

      i++;
    }
  }
}

} // namespace game2d