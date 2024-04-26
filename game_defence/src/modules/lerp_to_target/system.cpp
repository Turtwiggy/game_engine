#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/ai_pathfinding/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include "imgui.h" // hack
// #include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_set_velocity_to_target_system(entt::registry& r, const float& dt)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  // set target to next in generated path
  ImGui::Begin("DebugPathfinding");
  const auto& pathfinding = r.view<const AABB, HasTargetPositionComponent, GeneratedPathComponent>();
  for (const auto& [e, aabb, target, path] : pathfinding.each()) {
    ImGui::Text("Path Size: %i", path.path.size());

    const auto& cur = aabb.center;
    auto cur_gridpos = engine::grid::world_space_to_grid_space(glm::vec2(cur.x, cur.y), map.tilesize);
    cur_gridpos.x = glm::clamp(cur_gridpos.x, 0, map.xmax - 1);
    cur_gridpos.y = glm::clamp(cur_gridpos.y, 0, map.ymax - 1);
    ImGui::Text("cur_gridpos: %i %i", cur_gridpos.x, cur_gridpos.y);

    // get target grid position at the time which the pathfinding was calculated
    const auto dst_static = path.dst_pos;
    glm::ivec2 dst_gridpos_static = engine::grid::world_space_to_grid_space(dst_static, map.tilesize);
    dst_gridpos_static.x = glm::clamp(dst_gridpos_static.x, 0, map.xmax - 1);
    dst_gridpos_static.y = glm::clamp(dst_gridpos_static.y, 0, map.ymax - 1);
    ImGui::Text("dst_gridpos_static: %i %i", dst_gridpos_static.x, dst_gridpos_static.y);

    // try to get live target grid position
    // if no dst_ent is set, assume a static position
    std::optional<glm::ivec2> dst_gridpos_dynamic = std::nullopt;
    if (path.dst_ent == entt::null) {
      const auto dst_dynamic = r.get<AABB>(path.dst_ent).center;
      dst_gridpos_dynamic = engine::grid::world_space_to_grid_space(dst_dynamic, map.tilesize);
      dst_gridpos_dynamic.value().x = glm::clamp(dst_gridpos_dynamic.value().x, 0, map.xmax - 1);
      dst_gridpos_dynamic.value().y = glm::clamp(dst_gridpos_dynamic.value().y, 0, map.ymax - 1);
      ImGui::Text("dst_gridpos: %i %i", dst_gridpos_dynamic.value().x, dst_gridpos_dynamic.value().y);

      if (dst_gridpos_static != dst_gridpos_dynamic)
        ImGui::Text("Target has moved outside pathfinding??");
    }

    // probably clicked either on invalid spot, or spot in same gridspace
    const bool in_same_gridspace = cur_gridpos == dst_gridpos_dynamic;
    if ((path.path.size() == 0 || in_same_gridspace) && dst_gridpos_dynamic.has_value())
      target.position = r.get<AABB>(path.dst_ent).center; // dst_dynamic

    for (int i = 0; const auto p : path.path) {
      if (in_same_gridspace)
        continue; // probably shouldnt be in this for loop

      // not on our section in the path
      if (p != cur_gridpos) {
        i++;
        continue;
      }

      // move towards center of the current tile
      if (!path.path_cleared[i]) {
        const auto target_pos = engine::grid::grid_space_to_world_space(cur_gridpos, map.tilesize);
        target.position = target_pos;
        target.position += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);
      }

      // check if within distance of the center of the gridpos
      if (!path.path_cleared[i]) {
        const auto d = aabb.center - target.position;
        const float d2 = d.x * d.x + d.y * d.y;
        const float threshold = 10;
        if (d2 < threshold)
          path.path_cleared[i] = true;
      }

      // if we've cleared this current gridtile path,
      // aim for the next gridtile path
      if (path.path_cleared[i]) {
        const int next_idx = i + 1;
        const bool is_valid_next_index = next_idx <= path.path.size() - 1;
        if (is_valid_next_index) {
          const auto next_pos = path.path[next_idx];
          const auto target_pos = engine::grid::grid_space_to_world_space(next_pos, map.tilesize);
          // center, not top left
          target.position = target_pos;
          target.position += glm::vec2(map.tilesize / 2.0f, map.tilesize / 2.0f);
          break;
        }
      }

      i++;
    }

    // Debug path
    ImGui::Text("TargetPos: %i %i", target.position.x, target.position.y);
    ImGui::Separator();
    for (const auto& p : path.path)
      ImGui::Text("Path %i %i", p.x, p.y);
  }
  ImGui::End();

  //
  // If follow parent...
  // Set your target position as your parents target position.
  //
  const auto& follow_view = r.view<FollowTargetComponent, HasTargetPositionComponent>();
  for (const auto& [e, follow, target] : follow_view.each()) {
    const auto& p_pos = r.get<AABB>(follow.target);
    target.position = p_pos.center;
  }

  //
  // set velocity to target
  //
  const auto& view =
    r.view<VelocityComponent, const AABB, const HasTargetPositionComponent, const SetVelocityToTargetComponent>(
      entt::exclude<WaitForInitComponent>);
  for (const auto& [e, vel, aabb, target, lerp] : view.each()) {
    const glm::ivec2 a = { aabb.center.x, aabb.center.y };
    const glm::ivec2 b = target.position;
    const glm::vec2 nrm_dir = engine::normalize_safe(b - a);
    vel.x = nrm_dir.x * vel.base_speed;
    vel.y = nrm_dir.y * vel.base_speed;
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