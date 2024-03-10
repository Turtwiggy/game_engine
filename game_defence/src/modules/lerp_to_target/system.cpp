#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/ai_pathfinding/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "physics/components.hpp"

#include "imgui.h" // hack
// #include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_set_velocity_to_target_system(entt::registry& r, const float& dt)
{
  const auto& map = get_first_component<MapComponent>(r);

  ImGui::Begin("DebugPathfinding");

  // set target to next in generated path
  const auto& pathfinding = r.view<const AABB, HasTargetPositionComponent, GeneratedPathComponent>();
  for (const auto& [e, aabb, target, path] : pathfinding.each()) {
    const auto& cur = aabb.center;

    auto cur_gridpos = engine::grid::world_space_to_grid_space(glm::vec2(cur.x, cur.y), map.tilesize);
    cur_gridpos.x = glm::clamp(cur_gridpos.x, 0, map.xmax - 1);
    cur_gridpos.y = glm::clamp(cur_gridpos.y, 0, map.ymax - 1);
    ImGui::Text("CurrentGridPos: %i %i", cur_gridpos.x, cur_gridpos.y);
    ImGui::Text("Path Size: %i", path.path.size());

    // const auto cur_idx = engine::grid::grid_position_to_index(cur_gridpos, map.xmax);
    // const auto cur_idx_clamped = glm::clamp(cur_idx, 0, (map.xmax * map.ymax) - 1);

    // probably clicked either on invalid spot, or spot in same gridspace
    if (path.path.size() == 0)
      target.position = path.dst_pos;

    for (int i = 0; const auto p : path.path) {

      // not on our section in the path
      if (p != cur_gridpos) {
        i++;
        continue;
      }

      // cut corners
      path.path_cleared[i] = true;

      // move towards center of the current tile
      // if (!path.path_cleared[i]) {
      //   const auto target_pos = engine::grid::grid_space_to_world_space(cur_gridpos, map.tilesize);
      //   target.position = target_pos;
      //   target.position += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);
      // }

      // // check if within distance of the center of the gridpos
      // if (!path.path_cleared[i]) {
      //   const auto d = aabb.center - target.position;
      //   const float d2 = d.x * d.x + d.y * d.y;
      //   const float threshold = 10;
      //   if (d2 < threshold)
      //     path.path_cleared[i] = true;
      // }

      // if we've cleared this current gridtile path,
      // aim for the next gridtile path
      // if (path.path_cleared[i]) {
      //   const int next_idx = i + 1;
      //   const bool is_valid_next_index = next_idx <= path.path.size() - 1;
      //   if (is_valid_next_index) {
      //     const auto next_pos = path.path[next_idx];
      //     const auto target_pos = engine::grid::grid_space_to_world_space(next_pos, map.tilesize);
      //     // center, not top left
      //     target.position = target_pos;
      //     target.position += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);
      //   }
      // }

      // // if we're at the end of the path, aim for the original position
      // if (i == path.path.size() - 1)
      //   target.position = path.dst_pos;

      // skip all pathfinding...
      target.position = path.dst_pos;

      i++;
    }

    // Debug path
    ImGui::Separator();
    for (const auto& p : path.path)
      ImGui::Text("Path %i %i", p.x, p.y);
  }

  ImGui::End();

  // set velocity to target
  //
  const auto& view =
    r.view<VelocityComponent, const AABB, const HasTargetPositionComponent, const SetVelocityToTargetComponent>(
      entt::exclude<WaitForInitComponent>);
  for (const auto& [e, vel, aabb, target, lerp] : view.each()) {
    const glm::ivec2 a = { aabb.center.x, aabb.center.y };
    const glm::ivec2 b = target.position;
    const glm::vec2 nrm_dir = engine::normalize_safe(b - a);
    vel.x = nrm_dir.x * lerp.speed;
    vel.y = nrm_dir.y * lerp.speed;
  }

  //
  // This moves TransformComponent
  // That do not have AABB components.
  //
  const auto& t_view = r.view<VelocityComponent, TransformComponent>(entt::exclude<WaitForInitComponent, AABB>);
  for (const auto& [e, vel, t] : t_view.each()) {

    vel.remainder_x += vel.x * dt;
    vel.remainder_y += vel.y * dt;

    const int x = static_cast<int>(vel.remainder_x);
    const int y = static_cast<int>(vel.remainder_y);

    vel.remainder_x -= x;
    vel.remainder_y -= y;

    t.position.x += x;
    t.position.y += y;
  }
};

} // namespace game2d