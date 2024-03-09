#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "maths/maths.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/algorithm_dda/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_cursor_ui(entt::registry& r,
                 const CursorComponent& cursor_comp,
                 const bool click,
                 const bool held,
                 const bool release,
                 const glm::ivec2& mouse_pos,
                 const std::optional<glm::ivec2> click_position,
                 const std::optional<glm::ivec2> held_position)
{
  const auto& enemies = cursor_comp.hovering_enemies;

  // UI: Click action
  //
  if (click) {
    auto& click_t = r.emplace_or_replace<TransformComponent>(cursor_comp.click_ent);
    auto& held_t = r.emplace_or_replace<TransformComponent>(cursor_comp.held_ent);
    auto& line_t = r.emplace_or_replace<TransformComponent>(cursor_comp.line_ent);
    click_t.position = glm::ivec3(mouse_pos.x, mouse_pos.y, 0);

    // set scale, because transforms could be created for the first time
    click_t.scale = { 8, 8, 1 };
    held_t.scale = { 8, 8, 1 };
  }
  // UI: Held Action and not hovering enemies
  //
  if (held && enemies.size() == 0) {

    // position the held icon
    auto& held_t = r.get<TransformComponent>(cursor_comp.held_ent);
    held_t.position = glm::ivec3(mouse_pos.x, mouse_pos.y, 0);

    // position the line
    const auto line = generate_line(click_position.value(), held_position.value(), 2);
    auto& line_transform = r.get<TransformComponent>(cursor_comp.line_ent);
    set_transform_with_line(line_transform, line);
  }

  // UI: Release Action
  //
  if (release) {
    r.remove<TransformComponent>(cursor_comp.click_ent);
    r.remove<TransformComponent>(cursor_comp.held_ent);
    r.remove<TransformComponent>(cursor_comp.line_ent);
  }
}

void
attack_action(entt::registry& r, const entt::entity& enemy)
{
  // Tell all selected units to attack!
  //
  const auto& selected_view = r.view<SelectedComponent>(entt::exclude<WaitForInitComponent>);
  std::cout << "Setting targets for: " << selected_view.size_hint() << std::endl;

  for (const auto& [e, selected] : selected_view.each()) {
    //
    // set target as first enemy target
    r.get_or_emplace<DynamicTargetComponent>(e).target = enemy;

    // remove a static target if it existed
    if (r.try_get<StaticTargetComponent>(e)) {
      r.remove<StaticTargetComponent>(e);
    }
  }
}

void
move_action(entt::registry& r, const glm::ivec2& click_position, const glm::ivec2& held_position)
{
  const auto& map = get_first_component<MapComponent>(r);

  GridComponent grid;
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;

  // Position units along the line, facing the dir
  // const glm::vec2 raw_dir = held_position - click_position;
  // const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);
  // const glm::vec2 perp_dir = { -nrm_dir.y, nrm_dir.x };

  const auto& view = r.view<const SelectedComponent, const AABB, const HasTargetPositionComponent>();

  for (const auto& [e, selected, aabb, target_position] : view.each()) {
    const auto& src = aabb.center;
    const auto src_gridpos = engine::grid::world_space_to_grid_space(glm::vec2(src.x, src.y), map.tilesize);
    const auto src_idx = engine::grid::grid_position_to_index(src_gridpos, map.xmax);

    const auto& dst = click_position;
    const auto dst_gridpos = engine::grid::world_space_to_grid_space(glm::vec2(dst.x, dst.y), map.tilesize);
    const auto dst_idx = engine::grid::grid_position_to_index(dst_gridpos, map.ymax);

    // update the path
    auto path = generate_direct(r, grid, src_idx, dst_idx);

    GeneratedPathComponent path_c;
    path_c.path = path;
    path_c.src_pos = src;
    path_c.dst_pos = dst;
    path_c.path_cleared.resize(path.size());
    r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
  }

  // // Get unit info
  // //
  // int total_targets = 0;
  // float total_length = 0;
  // for (const auto& [e, selected, aabb, target] : selected_view.each()) {
  //   total_length += aabb.size.x;
  //   total_targets++;
  // }
  // const glm::vec2 total_targets_vec{ total_targets, total_targets };

  // const float size_of_each_unit = total_length / total_targets;
  // const glm::vec2 size{ size_of_each_unit, size_of_each_unit };

  // // Update selcted units target position
  // //
  // for (int i = 1; const auto& [e, selected, aabb, target_pos] : selected_view.each()) {

  //   glm::vec2 pos = click_position;

  //   pos += perp_dir * ((glm::vec2(i, i) * size) - (size / 2.0f) - (size * total_targets_vec / 2.0f));

  //   target_pos.position = pos;

  //   i++;
  // }
}

void
update_selected_interactions_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto& cursor = get_first<CursorComponent>(r);
  const auto& cursor_comp = r.get<CursorComponent>(cursor);
  const auto& enemies = cursor_comp.hovering_enemies;
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // warning: doesnt work with controller currently
  const bool click = get_mouse_rmb_press();
  const bool held = get_mouse_rmb_held();
  const bool release = get_mouse_rmb_release();
  const bool ctrl_held = get_key_held(input, SDL_SCANCODE_LCTRL);

  static std::optional<glm::ivec2> click_position;
  static std::optional<glm::ivec2> held_position;
  if (click)
    click_position = mouse_pos;
  if (held)
    held_position = mouse_pos;

  update_cursor_ui(r, cursor_comp, click, held, release, mouse_pos, click_position, held_position);

  bool valid_move_action_for_hovered_object = false;
  {
    ImGui::Begin("SelectedInteractions");

    const auto& map = get_first_component<MapComponent>(r);
    const auto& map_tilesize = map.tilesize;
    ImGui::Text("regular %i %i", mouse_pos.x, mouse_pos.y);

    const auto mouse_gridspace = engine::grid::world_space_to_grid_space(glm::vec2(mouse_pos.x, mouse_pos.y), map_tilesize);
    ImGui::Text("gridspace %i %i", mouse_gridspace.x, mouse_gridspace.y);

    // check out of bounds
    const float min_x = 0;
    const float max_x = map.xmax * map.tilesize;
    const float min_y = 0;
    const float max_y = map.ymax * map.tilesize;
    const auto within_range = [](const float v, const float a, const float b) { return (v >= a && v <= b); };
    if (within_range(mouse_pos.x, min_x, max_x) && within_range(mouse_pos.y, min_y, max_y)) {
      const auto mouse_gridspace_idx = engine::grid::grid_position_to_index(mouse_gridspace, map.xmax);
      const auto mouse_gridspace_idx_clamped = glm::clamp(mouse_gridspace_idx, 0, (map.xmax * map.ymax) - 1);
      const auto& map_entries = map.map[mouse_gridspace_idx_clamped];
      for (const auto& path_e : map_entries) {
        const auto& pathfind = r.get<PathfindComponent>(path_e);
        ImGui::Text("Hovered object pathfinding cost: %i", pathfind.cost);
        if (pathfind.cost != -1)
          valid_move_action_for_hovered_object = true;
      }
    } else
      ImGui::Text("Mouse out of grid");
    ImGui::End();
  }

  //
  // ctrl+click
  // create a line between the player and the selected location
  // if that line contains or collides with an enemy,
  // the player should shoot
  //
  if (ctrl_held && click) {

    const auto& selected_view =
      r.view<PlayerComponent, const SelectedComponent, const AABB>(entt::exclude<WaitForInitComponent>);
    for (const auto& [e, player, selected, aabb] : selected_view.each()) {
      const auto line_e = create_gameplay(r, EntityType::empty_with_physics);
      r.emplace<LineOfSightComponent>(line_e);
      r.emplace<HasParentComponent>(line_e, e); // set player as parent

      // update target
      r.emplace_or_replace<StaticTargetComponent>(e, mouse_pos);
      if (r.try_get<DynamicTargetComponent>(e))
        r.remove<DynamicTargetComponent>(e);

      const auto line_info = generate_line(aabb.center, mouse_pos, 10);
      auto& line_transform = r.get<TransformComponent>(line_e);
      set_transform_with_line(line_transform, line_info);
      auto& line_aabb = r.get<AABB>(line_e);
      line_aabb.center = line_transform.position;
      line_aabb.size = { line_transform.scale.x, line_transform.scale.y };

      if (player.weapon_line_of_sight != entt::null) {
        // remove the old line of sight
        dead.dead.emplace(player.weapon_line_of_sight);
      }
      // set the new line of sight
      player.weapon_line_of_sight = line_e;
    }

  }
  //
  // click + attack
  //
  else if (click && enemies.size() > 0) {
    attack_action(r, enemies[0]);
  }
  //
  // release + move
  //
  else if (release && enemies.size() == 0 && valid_move_action_for_hovered_object) {
    move_action(r, click_position.value(), held_position.value());

    click_position = std::nullopt;
    held_position = std::nullopt;
  }
}

} // namespace game2d

///
// TRY OUT DDA
///

// HACK: dda start
// const auto& players = r.view<PlayerComponent, TransformComponent>();
// if (players.size_hint() == 0)
//   return;
// // entt::entity first_player = entt::null;
// TransformComponent first_player_t;
// for (const auto& [e, player, t] : players.each()) {
//   // first_player = e;
//   first_player_t = t;
//   break;
// }

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
