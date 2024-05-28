#include "system.hpp"

#include "components.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"

#include "imgui.h"
#include "sprites/helpers.hpp"

namespace game2d {
using namespace std::literals;

// check if in range
// if in range,
// give all entities in the fight an initiative.
// some entities may join and leave combat
// update_combat_engagement_system(r);
// update_combat_turnbased_system(r);

enum class CursorType
{
  EMPTY,
  MOVE,
  ATTACK,
};

enum class ActionMode
{
  MOVE,
  ATTACK,
};

void
change_cursor(entt::registry& r, const CursorType& type)
{
  const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& e = info.action_cursor;
  const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;
  const int tex_unit_icons = search_for_texture_unit_by_texture_path(ri, "gameicons")->unit;

  if (type == CursorType::EMPTY) {
    set_size(r, e, { 0, 0 });
    set_sprite_custom(r, e, "EMPTY"s, tex_unit_kenny);
  } else if (type == CursorType::ATTACK) {
    set_size(r, e, { 16, 16 });
    set_sprite_custom(r, e, "CURSOR_ATTACK_2"s, tex_unit_kenny);
  } else if (type == CursorType::MOVE) {
    set_size(r, e, { 16, 16 });
    set_sprite_custom(r, e, "ARROW_BOTTOM_RIGHT"s, tex_unit_icons);
  }
};

void
update_ui_combat_turnbased_system(entt::registry& r, const glm::ivec2& input_mouse_pos)
{
  const int mouse_grid_increments = 5;

  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map = get_first_component<MapComponent>(r);

  const auto& data_e = get_first<SINGLE_DuckgameToDungeon>(r);
  if (data_e != entt::null) {
    // you arrived from a scene
  }

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  static int action = 1;
  {
    const bool action_1 = get_key_down(input, SDL_Scancode::SDL_SCANCODE_1);
    const bool action_2 = get_key_down(input, SDL_Scancode::SDL_SCANCODE_2);
    if (action_1)
      action = 1;
    if (action_2)
      action = 2;
  }

  const bool rmb_click = get_mouse_rmb_press();

  const int grid_snap_size = mouse_grid_increments; // note: this is not the map.tilesize,
  glm::ivec2 mouse_pos = engine::grid::grid_space_to_world_space(
    engine::grid::world_space_to_grid_space(input_mouse_pos, grid_snap_size), grid_snap_size);
  mouse_pos += glm::vec2(grid_snap_size / 2.0f, grid_snap_size / 2.0f); // center

  // Convert Map to Grid (?)
  GridComponent grid;
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;

  const auto gridpos = engine::grid::world_space_to_grid_space(input_mouse_pos, map.tilesize);
  const auto clamped_gridpos = engine::grid::world_space_to_grid_space(mouse_pos, map.tilesize);

  ImGui::Begin("UI Combat");
  ImGui::Text("MousePos: %i %i", input_mouse_pos.x, input_mouse_pos.y);
  ImGui::Text("MousePos GridPos: %i %i", gridpos.x, gridpos.y);
  ImGui::Text("MouseClampedPos: %i %i", mouse_pos.x, mouse_pos.y);
  ImGui::Text("MousePos ClampedGridPos: %i %i", clamped_gridpos.x, clamped_gridpos.y);
  ImGui::Text("Action: %i", action);

  // Hack: debug gridpos pathfinding
  {
    const auto grid_idx = engine::grid::grid_position_to_clamped_index(gridpos, map.xmax, map.ymax);
    if (map.map[grid_idx].size() > 0) {
      for (const auto& map_e : map.map[grid_idx]) {
        if (const auto* pfc = r.try_get<PathfindComponent>(map_e))
          ImGui::Text("Cost: %i", pfc->cost);
        else
          ImGui::Text("Entity without pathfinding component");
      }
    }
  }

  // Hack: display all units hp in worldspace
  {
    const auto& view = r.view<HealthComponent, AABB>();
    for (const auto& [e, hp, aabb] : view.each()) {
      auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
      worldspace_ui.text = std::to_string(hp.hp);
      worldspace_ui.offset.y = -aabb.size.y / 2.0f;
    }
  }

  ImGui::Text("Hello, Combat!");

  // Check if all enemies are dead.
  //
  const auto& enemy_view = r.view<EnemyComponent>();
  const int enemy_count = enemy_view.size();
  ImGui::Text("Enemies: %i", enemy_count);
  if (enemy_count == 0) {
    if (ImGui::Button("Back to Overworld"))
      move_to_scene_start(r, Scene::duckgame_overworld, true);
  }

  const auto& selected_view = r.view<SelectedComponent, AABB>();
  int count = 0;
  for (const auto& [e, selected_c, aabb] : selected_view.each())
    count++;
  ImGui::Text("Count: %i", count);

  // limit to only interacting with 1 selected unit
  if (count != 1) {
    ImGui::End();
    return;
  }

  // set positon of cursor
  const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
  set_position(r, info.action_cursor, mouse_pos);

  for (const auto& [e, selected_c, aabb] : selected_view.each()) {

    const auto src = aabb.center;
    const auto src_idx = convert_position_to_index(map, src);
    const auto src_gridpos = engine::grid::index_to_grid_position(src_idx, map.xmax, map.ymax);

    const auto dst = mouse_pos;
    const int dst_idx = convert_position_to_index(map, dst);

    // bugfix: only go to destination if the end-tile isn't blocked
    // check the destination is traversable
    bool traversable = true;
    if (map.map[dst_idx].size() > 0) {
      for (const auto& e : map.map[dst_idx]) {
        if (const auto* cost = r.try_get<PathfindComponent>(e)) {
          if (cost->cost == -1)
            traversable = false;
        }
      }
    }
    ImGui::Text("Traversable: %i", traversable);

    // Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
    const auto update_path_to_mouse = [&r, &e, &grid, &src_idx, &src, &dst_idx, &dst, &src_gridpos]() {
      if (auto* existing_path = r.try_get<GeneratedPathComponent>(e)) {
        if (existing_path->path.size() > 0) {
          const auto last = existing_path->path[existing_path->path.size() - 1];
          if (last != src_gridpos)
            return; // already has a path that you've not yet arrived at
        }
      }
      const auto path = generate_direct(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.dst_ent = entt::null;
      path_c.aim_for_exact_position = true;
      path_c.required_to_clear_path = true;
      path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    // aim gun
    auto& static_tgt = r.get_or_emplace<StaticTargetComponent>(e);
    static_tgt.target = { mouse_pos.x, mouse_pos.y };

    // move mode
    if (action == 1) {
      change_cursor(r, CursorType::MOVE);
      if (rmb_click && traversable)
        update_path_to_mouse();
    }

    // shoot mode
    if (action == 2) {
      change_cursor(r, CursorType::ATTACK);
      if (rmb_click)
        r.emplace_or_replace<WantsToShoot>(e);
    }
  }

  ImGui::End();
}

} // namespace game2d