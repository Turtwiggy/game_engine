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
#include "modules/grid/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

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

  const auto gridpos = engine::grid::world_space_to_grid_space(input_mouse_pos, map.tilesize);
  const auto grid_idx = engine::grid::grid_position_to_index(gridpos, map.xmax);

  ImGui::Begin("UI Combat");
  ImGui::Text("MousePos: %i %i", input_mouse_pos.x, input_mouse_pos.y);
  ImGui::Text("MousePos GridPos: %i %i", gridpos.x, gridpos.y);
  ImGui::Text("MouseClampedPos: %i %i", mouse_pos.x, mouse_pos.y);
  ImGui::Text("Mouse GridIndex: %i", grid_idx);
  ImGui::Text("Action: %i", action);

  // check if all enemies are ded.
  std::map<AvailableTeams, int> team_count;
  team_count[AvailableTeams::player] = 0; // force team to exist
  team_count[AvailableTeams::enemy] = 0;  // force team to exist
  for (const auto& [e, team_c] : r.view<TeamComponent>().each())
    team_count[team_c.team] += 1; // count teams
  if (team_count[AvailableTeams::enemy] == 0) {
    if (ImGui::Button("You won! Back to overworld"))
      move_to_scene_start(r, Scene::duckgame_overworld, true);
  }

  auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team == AvailableTeams::neutral)
    if (ImGui::Button("Swap to player turn"))
      state.team = AvailableTeams::player;
  const auto type_name = std::string(magic_enum::enum_name(state.team));
  ImGui::Text("Turn: %s", type_name.c_str());

  // Hack: debug gridpos pathfinding
  {
    const auto gridpos = engine::grid::world_space_to_grid_space(input_mouse_pos, map.tilesize);
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

  const auto& selected_view = r.view<SelectedComponent, AABB>();
  int count = 0;
  for (const auto& [e, selected_c, aabb] : selected_view.each())
    count++;

  if (ImGui::Button("End Turn")) // let player end their turn
    r.emplace<RequestToCompleteTurn>(r.create(), AvailableTeams::player);

  // limit: must be interacting with 1 selected unit
  if (count != 1) {
    ImGui::End();
    return;
  }

  // limit: must be player turn
  if (state.team != AvailableTeams::player) {
    ImGui::End();
    return;
  }

  // set positon of cursor
  const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
  set_position(r, info.action_cursor, mouse_pos);

  for (const auto& [e, selected_c, aabb] : selected_view.each()) {

    // aim gun
    auto& static_tgt = r.get_or_emplace<StaticTargetComponent>(e);
    static_tgt.target = { mouse_pos.x, mouse_pos.y };

    auto& turn_state = r.get_or_emplace<TurnState>(e);
    const bool has_moved = turn_state.has_moved;
    const bool has_shot = turn_state.has_shot;

    ImGui::Separator();
    ImGui::Text("has_moved %i", has_moved);
    ImGui::Text("has_shot: %i", has_shot);

    // already has a path that you've not yet arrived at
    const bool do_move = !has_destination(r, e) || at_destination(r, e);

    // move mode
    if (action == 1 && !has_moved && do_move) {
      change_cursor(r, CursorType::MOVE);
      if (rmb_click) {
        update_path_to_mouse(r, e, mouse_pos);
        turn_state.has_moved = true;
      }
    }

    // shoot mode
    if (action == 2 && !has_shot) {
      change_cursor(r, CursorType::ATTACK);
      if (rmb_click) {
        r.emplace_or_replace<WantsToShoot>(e);
        turn_state.has_shot = true;
      }
    }
  }

  ImGui::End();
}

} // namespace game2d