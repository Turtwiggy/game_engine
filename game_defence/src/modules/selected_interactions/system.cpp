#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <imgui.h>

#include <optional>

namespace game2d {

void
update_selected_interactions_system(entt::registry& r, const glm::ivec2& mouse_pos, const float dt)
{
  // aim gun
  //
  const auto& player_view = r.view<PlayerComponent, AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, player, player_aabb] : player_view.each()) {
    auto& static_tgt = r.get_or_emplace<StaticTargetComponent>(e);
    static_tgt.target = { mouse_pos.x, mouse_pos.y };
  }
}

} // namespace game2d

// HACK: dda start
// const auto& players = r.view<PlayerComponent, TransformComponent>();
// const glm::vec2 tile_vec = glm::vec2(map.tilesize, map.tilesize);
// const glm::vec2 mouse_cell = glm::vec2(mouse_pos.x, mouse_pos.y) / tile_vec;
// const glm::vec2 player_cell = glm::vec2(first_player_t.position.x, first_player_t.position.y) / tile_vec;
// DDAInput dda_input;
// dda_input.start = player_cell;
// dda_input.end = mouse_cell;
// dda_input.map_size = { map.xmax, map.ymax };
// const auto output = dda_raycast(dda_input, map.map);
// auto& dda_start = r.get<TransformComponent>(cursor_comp.dda_start);
// auto& dda_end = r.get<TransformComponent>(cursor_comp.dda_end);
// dda_start.position = { dda_input.start.x * tile_vec.x, dda_input.start.y * tile_vec.y, 0.0f };
// dda_end.position = { dda_input.end.x * tile_vec.x, dda_input.end.y * tile_vec.x, 0.0f };
// auto& dda_intersection = r.get<TransformComponent>(cursor_comp.dda_intersection);
// auto& dda_intersection_spr = r.get<SpriteComponent>(cursor_comp.dda_intersection);
// dda_intersection.position = { output.intersection.x * tile_vec.x, output.intersection.y * tile_vec.y, 0.0f };
// dda_intersection_spr.colour = engine::LinearColour(1.0f, 0.0f, 0.0f, 1.0f);

// HACK: pathfinding start
// const auto& map = get_first_component<MapComponent>(r);
// GridComponent grid;
// grid.size = map.tilesize;
// grid.width = map.xmax;
// grid.height = map.ymax;
// grid.grid = map.map;
// const auto& view = r.view<const SelectedComponent, const AABB, HasTargetPositionComponent>();
// for (const auto& [e, selected, aabb, target_position] : view.each()) {
//   const auto& src = aabb.center;
//   const auto src_gridpos = engine::grid::world_space_to_grid_space(glm::vec2(src.x, src.y), map.tilesize);
//   const auto src_idx = engine::grid::grid_position_to_index(src_gridpos, map.xmax);
//   const auto& dst = click_position;
//   const auto dst_gridpos = engine::grid::world_space_to_grid_space(glm::vec2(dst.x, dst.y), map.tilesize);
//   const auto dst_idx = engine::grid::grid_position_to_index(dst_gridpos, map.ymax);
//   // update the path
//   auto path = generate_direct(r, grid, src_idx, dst_idx);
//   GeneratedPathComponent path_c;
//   path_c.path = path;
//   path_c.src_pos = src;
//   path_c.dst_pos = dst;
//   path_c.path_cleared.resize(path.size());
//   r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
// }

// HACK: set aim waypoint
//
// ctrl+click
// create a line between the player and the selected location
// if that line contains or collides with an enemy,
// the player should shoot
//
// if (ctrl_held && click) {
//   const auto& selected_view =
//     r.view<PlayerComponent, const SelectedComponent, const AABB>(entt::exclude<WaitForInitComponent>);
//   for (const auto& [e, player, selected, aabb] : selected_view.each()) {
//     const auto line_e = create_gameplay(r, EntityType::empty_with_physics);
//     r.emplace<LineOfSightComponent>(line_e);
//     r.emplace<HasParentComponent>(line_e, e); // set player as parent
//     // update target
//     r.emplace_or_replace<StaticTargetComponent>(e, mouse_pos);
//     if (r.try_get<DynamicTargetComponent>(e))
//       r.remove<DynamicTargetComponent>(e);
//     const auto line_info = generate_line(aabb.center, mouse_pos, 10);
//     auto& line_transform = r.get<TransformComponent>(line_e);
//     set_transform_with_line(line_transform, line_info);
//     auto& line_aabb = r.get<AABB>(line_e);
//     line_aabb.center = line_transform.position;
//     line_aabb.size = { line_transform.scale.x, line_transform.scale.y };
//     if (player.weapon_line_of_sight != entt::null) {
//       dead.dead.emplace(player.weapon_line_of_sight);
//     // set the new line of sight
//     player.weapon_line_of_sight = line_e;
//   }
// }