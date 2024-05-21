#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

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

// void
// set_hover_debug(entt::registry& r, const bool& enabled, const glm::ivec2& position, const glm::ivec2& size)
// {
//   const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);

//   // if (enabled) {
//   // r.emplace_or_replace<TransformComponent>(info.to_place_debug);
//   // set_position(r, info.to_place_debug, position);
//   // set_size(r, info.to_place_debug, size);
//   // } else
//   {
//     if (auto* transform = r.try_get<TransformComponent>(info.to_place_debug))
//       r.remove<TransformComponent>(info.to_place_debug);
//   }
// };

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
  const int mouse_grid_increments = 10;

  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map = get_first_component<MapComponent>(r);

  const auto& data_e = get_first<SINGLE_DuckgameToDungeon>(r);
  if (data_e != entt::null) {
    // you arrived from a scene
  }

  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const bool left_ctrl_held = get_key_held(input, SDL_Scancode::SDL_SCANCODE_LCTRL);
  const bool left_shift_held = get_key_held(input, SDL_Scancode::SDL_SCANCODE_LSHIFT);
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

  // HACK: BAD: clear entity map.
  {
    for (int i = 0; i < map.map.size(); i++) {
      std::vector<entt::entity>& ents = map.map[i];
      ents.clear();
    }
  }

  // HACK: BAD: update the entity map with costs.
  {
    const auto& view = r.view<PathfindComponent, AABB>();
    for (const auto& [e, pc, aabb] : view.each()) {
      //
      // work out which gridpos this object is in
      // check directions l, r, u, d
      const float half_x = aabb.size.x / 2.0f;
      const float half_y = aabb.size.y / 2.0f;
      std::set<vec2i> gridcells;
      for (int y = aabb.center.y - half_y; y <= aabb.center.y + half_y; y += map.tilesize)
        for (int x = aabb.center.x - half_x; x <= aabb.center.x + half_x; x += map.tilesize)
          gridcells.emplace(vec2i(engine::grid::world_space_to_grid_space({ x, y }, map.tilesize)));
      for (const auto& gc : gridcells) {
        // ImGui::Text("GridCell %i %i", gc.x, gc.y);
        const int idx = engine::grid::grid_position_to_clamped_index({ gc.x, gc.y }, map.xmax, map.ymax);
        map.map[idx].push_back(e);
      }
    }
  }

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
    const auto& view = r.view<HealthComponent>();
    for (const auto& [e, hp] : view.each()) {
      auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
      worldspace_ui.text = std::to_string(hp.hp);
    }
  }

  ImGui::Text("Hello, Combat!");

  // Check if all enemies are dead.
  //
  const auto& enemy_view = r.view<EnemyComponent>();
  int enemy_count = 0;
  for (const auto& [e, e_c] : enemy_view.each())
    enemy_count++;
  if (enemy_count == 0) {
    if (ImGui::Button("Back to Overworld")) {
      ImGui::Text("All enemies dead");

      move_to_scene_start(r, Scene::duckgame_overworld, true);
    }
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

    const auto dst = mouse_pos;
    const int dst_idx = convert_position_to_index(map, dst);

    // Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
    const auto update_path_to_mouse = [&r, &e, &grid, &src_idx, &src, &dst_idx, &dst]() {
      const auto path = generate_direct_with_diagonals(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.dst_ent = entt::null;
      path_c.aim_for_exact_position = true;
      // path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    // aim gun
    auto& static_tgt = r.get_or_emplace<StaticTargetComponent>(e);
    static_tgt.target = { mouse_pos.x, mouse_pos.y };

    // default cursor
    change_cursor(r, CursorType::MOVE);

    // shoot mode
    if (left_ctrl_held) {
      change_cursor(r, CursorType::ATTACK);
      if (rmb_click)
        r.emplace_or_replace<WantsToShoot>(e);
    }

    // move
    else if (rmb_click)
      update_path_to_mouse();
  }

  ImGui::End();
}

} // namespace game2d