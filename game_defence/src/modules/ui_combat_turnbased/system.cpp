#include "system.hpp"

#include "components.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
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
#include "modules/system_turnbased/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_event_console/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

#include "imgui.h"
#include <climits>

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
  const auto& map = get_first_component<MapComponent>(r);
  const auto& e = info.action_cursor;
  const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;
  const int tex_unit_icons = search_for_texture_unit_by_texture_path(ri, "gameicons")->unit;

  if (type == CursorType::EMPTY) {
    set_size(r, e, { 0, 0 });
    set_sprite_custom(r, e, "EMPTY"s, tex_unit_kenny);
  } else if (type == CursorType::MOVE) {
    set_size(r, e, { map.tilesize, map.tilesize });
    set_sprite_custom(r, e, "CURSOR_1"s, tex_unit_kenny);
  } else if (type == CursorType::ATTACK) {
    set_size(r, e, { 16, 16 });
    set_sprite_custom(r, e, "CURSOR_ATTACK_2"s, tex_unit_kenny);
  }
};

void
update_ui_combat_turnbased_system(entt::registry& r, const glm::ivec2& input_mouse_pos)
{

  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  auto& map = get_first_component<MapComponent>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  static ActionMode action = ActionMode::MOVE;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_1))
    action = ActionMode::MOVE;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_2))
    action = ActionMode::ATTACK;

  int mouse_grid_increments = 1;
  if (action == ActionMode::ATTACK)
    mouse_grid_increments = 1;
  if (action == ActionMode::MOVE)
    mouse_grid_increments = map.tilesize;

  const bool rmb_click = get_mouse_rmb_press();
  const int grid_snap_size = mouse_grid_increments; // note: this is not the map.tilesize,

  const auto mouse_gridspace = engine::grid::world_space_to_grid_space(input_mouse_pos, grid_snap_size);
  auto mouse_pos = engine::grid::grid_space_to_world_space(mouse_gridspace, grid_snap_size);
  if (action == ActionMode::MOVE)
    mouse_pos += glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f }; // center it

  // change cursor icon
  if (action == ActionMode::MOVE)
    change_cursor(r, CursorType::MOVE);
  if (action == ActionMode::ATTACK)
    change_cursor(r, CursorType::ATTACK);

  // Show combat info menu
  {
    const auto& state = get_first_component<SINGLE_CombatState>(r);
    if (state.team == AvailableTeams::neutral) {
      const auto& viewport = ImGui::GetWindowViewport();
      ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5, 0.5));
      ImGui::SetNextWindowSize(ImVec2{ 400, 200 });

      ImGuiWindowFlags flags = 0;
      flags |= ImGuiWindowFlags_NoFocusOnAppearing;
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoCollapse;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoBackground;
      flags |= ImGuiDockNodeFlags_NoResize;
      flags |= ImGuiDockNodeFlags_PassthruCentralNode;

      ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
      ImGui::Begin("Begin Combat", NULL, flags);
      {
        // ImGui::Text("- Press Q and E to swap between your units.");
        // ImGui::Text("- Press 1 for move action. Press 2 for shoot action.");
        // ImGui::Text("- Right click at location to execute action.");
        // ImGui::Separator();
        // ImGui::Text("- Defeat all enemies to win. Good Luck!");

        // Center button horizontally
        // ImGuiStyle& style = ImGui::GetStyle();

        // calculate reqiured x-spacing
        // float width = 0.0f;
        // width += ImGui::CalcTextSize(label.c_str()).x;
        // width += style.ItemSpacing.x;

        // AlignForWidth()
        // const float alignment = 0.5f;
        // const float avail = ImGui::GetContentRegionAvail().x;
        // float off = (avail - width) * alignment;
        // if (off > 0.0f)
        //   ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

        // Draw the button(s)
        const ImVec2 button_size = ImGui::GetContentRegionAvail();
        if (ImGui::Button("Begin Combat", button_size)) {
          auto& state = get_first_component<SINGLE_CombatState>(r);
          state.team = AvailableTeams::player;
        }

        // HACK: hide this menu
        // state.team = AvailableTeams::player;

        ImGui::End();

        ImGui::PopStyleVar();
        return; // must click begin!
      }
    }
  }

#if defined(_DEBUG)
  ImGui::Begin("UI Combat");
  ImGui::Text("MousePos: %i %i", input_mouse_pos.x, input_mouse_pos.y);

  const auto gridpos = engine::grid::world_space_to_grid_space(input_mouse_pos, map.tilesize);
  ImGui::Text("MousePos GridPos: %i %i", gridpos.x, gridpos.y);
  ImGui::Text("MouseClampedPos: %i %i", mouse_pos.x, mouse_pos.y);

  const auto grid_idx = engine::grid::grid_position_to_index(gridpos, map.xmax);
  ImGui::Text("Mouse GridIndex: %i", grid_idx);
  ImGui::Text("Action: %i", action);

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

  ImGui::End();
#endif

  // check if all enemies are ded.
  std::map<AvailableTeams, int> team_count;
  team_count[AvailableTeams::player] = 0; // force team to exist
  team_count[AvailableTeams::enemy] = 0;  // force team to exist
  for (const auto& [e, team_c] : r.view<TeamComponent>().each())
    team_count[team_c.team] += 1; // count teams
  if (team_count[AvailableTeams::enemy] == 0) {
    const bool you_came_here_from_overworld = get_first<OverworldToDungeonInfo>(r) != entt::null;
    if (you_came_here_from_overworld) {

      // center this window
      const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
      const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
      const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
      const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
      ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200 * (9 / 16.0f)), ImVec2(200, 200 * (9 / 16.0f)));

      ImGuiWindowFlags flags = 0;
      // position and sizing
      flags |= ImGuiWindowFlags_NoMove;
      flags |= ImGuiWindowFlags_NoCollapse;
      flags |= ImGuiWindowFlags_NoDocking;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoFocusOnAppearing;
      // visuals
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoBackground;

      ImGui::Begin("Back To Overworld", NULL, flags);
      if (ImGui::Button("Spaceship clear. Back to overworld", ImVec2{ 200, 100 })) {

        // add an event because fun
        auto& evt = get_first_component<SINGLE_EventConsoleLogComponent>(r);
        evt.events.push_back("Spaceship cleared.");

        move_to_scene_start(r, Scene::overworld, true);
        ImGui::End();
        return;
      }
    }
  }

  // Hack: display all units hp/defence in worldspace
  {
    const auto& view = r.view<HealthComponent, DefenceComponent, AABB>();
    for (const auto& [e, hp, defence, aabb] : view.each()) {
      auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
      worldspace_ui.text = std::to_string(hp.hp) + " " + std::to_string(defence.armour);
      worldspace_ui.offset.y = -aabb.size.y;
    }
  }

  const auto& selected_view = r.view<SelectedComponent, AABB>();
  int count = 0;
  for (const auto& [e, selected_c, aabb] : selected_view.each())
    count++;

  // stop showing movement path
  if (count == 0 || action != ActionMode::MOVE) {
    auto& state = get_first_component<SINGLE_CombatState>(r);
    state.show_selected_player_path.update(r, 0);
  }

  // set positon of cursor
  const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
  set_position(r, info.action_cursor, mouse_pos);

  // move all guns to the mouse cursor
  for (const auto& [e, selected_c, aabb_c] : selected_view.each()) {
    auto& static_tgt = r.get_or_emplace<StaticTargetComponent>(e);
    static_tgt.target = { mouse_pos.x, mouse_pos.y };
  }

  // limit: must be interacting with 1 selected unit
  if (count != 1)
    return;

  // limit: must be player turn
  auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team != AvailableTeams::player)
    return;

  for (const auto& [e, selected_c, aabb] : selected_view.each()) {

    // UX: show the path the unit would take
    if (action == ActionMode::MOVE) {
      // Generate full path for ux
      const auto path = generate_path(r, e, mouse_pos, INT_MAX);
      state.show_selected_player_path.update(r, path.size());

      const auto limit = r.get<MoveLimitComponent>(e).amount;
      for (int i = 0; const auto& p : path) {
        const auto& ui_e = state.show_selected_player_path.instances[i];
        set_position_grid(r, ui_e, p);
        set_size(r, ui_e, { 6, 6 });
        set_sprite(r, ui_e, "EMPTY");

        // show full path, but make it red (i.e. you're not gonna move that far)
        if (i > limit)
          set_colour(r, ui_e, { 1.0f, 0.0, 0.0, 1.0f });

        i++;
      }
    }

    auto& turn_state = r.get_or_emplace<TurnState>(e);
    const bool has_moved = turn_state.completed_move;
    const bool has_shot = turn_state.completed_shot;

    // already has a path that you've not yet arrived at
    bool do_move = !has_destination(r, e) || at_destination(r, e);

    // destination must be free
    do_move &= !destination_is_blocked(r, mouse_pos);

    // move mode
    if (action == ActionMode::MOVE && !has_moved && do_move) {
      if (rmb_click) {
        const auto limit = r.get<MoveLimitComponent>(e).amount;
        const auto path = generate_path(r, e, mouse_pos, limit);
        update_entity_path(r, e, path);
        turn_state.completed_move = true;
      }
    }

    // shoot mode
    if (action == ActionMode::ATTACK && !has_shot) {
      if (rmb_click) {
        r.emplace_or_replace<WantsToShoot>(e);
        turn_state.completed_shot = true;
      }
    }
  }
}

} // namespace game2d