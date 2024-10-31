#include "ui_action_bar_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/ui_combat_designer/ui_combat_designer_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_scancode.h>
#include <imgui.h>
#include <magic_enum.hpp>

namespace game2d {

enum class UnitAction
{
  NONE,
  MOVE,
  SHOOT,
  USE_ITEM,
};

struct UnitActionState
{
  UnitAction current = UnitAction::NONE;
};

struct UnitCompletedActions
{
  std::vector<UnitAction> actions;
};

void
update_ui_action_bar_system(entt::registry& r, const glm::ivec2 mouse_pos)
{
  const auto init_e = get_first<SINGLE_Initiative>(r);
  if (init_e == entt::null)
    return;
  const auto& init_c = r.get<SINGLE_Initiative>(init_e);
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  const auto move_action = SDL_SCANCODE_1;
  const auto shoot_action = SDL_SCANCODE_2;

  if (init_c.order.size() == 0)
    return; // no units with initiative

  // it's the first unit's turn...
  const auto e = init_c.order[0];

  ImGui::Begin("Action Bar");

  if (ImGui::Button("Clear In P")) {
    if (r.try_get<UnitActionState>(e))
      r.remove<UnitActionState>(e);
    if (r.try_get<GeneratedPathComponent>(e))
      r.remove<GeneratedPathComponent>(e);
  }

  if (ImGui::SameLine(); ImGui::Button("Move") || get_key_down(input_c, move_action))
    r.emplace_or_replace<UnitActionState>(e, UnitAction::MOVE);

  if (ImGui::SameLine(); ImGui::Button("Shoot") || get_key_down(input_c, shoot_action))
    r.emplace_or_replace<UnitActionState>(e, UnitAction::SHOOT);

  if (ImGui::SameLine(); ImGui::Button("Use"))
    r.emplace_or_replace<UnitActionState>(e, UnitAction::USE_ITEM);

  const bool allowed_to_end = !has_destination(r, e);
  ImVec4 col_active = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  ImVec4 col_inactive = ImVec4(0.8f, 0.5f, 0.5f, 1.0f);
  ImVec4 col = col_active;
  if (!allowed_to_end)
    col = col_inactive;

  if (ImGui::SameLine(); ImGui::ColorButton("End", col) && allowed_to_end) {

    // .. remove in progress action
    if (r.try_get<UnitActionState>(e))
      r.remove<UnitActionState>(e);

    // .. remove completed actions
    if (r.try_get<UnitCompletedActions>(e))
      r.remove<UnitCompletedActions>(e);

    // Set the initiative of the first unit to one higher than the last unit
    if (init_c.order.size() > 1) {
      const auto last_e = init_c.order[init_c.order.size() - 1];
      const auto& last_c = r.get<InitiativeComponent>(last_e);
      auto& first_c = r.get<InitiativeComponent>(e);
      // This line here is weird. It probably shouldnt be +1,
      // but should be relative to the units own agility or dexterity.
      // This means that faster units would take turns more frequently.
      first_c.initiative = last_c.initiative + 1;

      const auto next_e = init_c.order[1];
      activate_unit(r, next_e);
    }
  }

  //
  // State impl
  //

  if (auto* state_c = r.try_get<UnitActionState>(e)) {

    bool request_action = false;
    if (auto* inp_c = r.try_get<InputComponent>(e))
      request_action |= inp_c->shoot;

    // no ui
    request_action &= (ri.viewport_hovered);

    const auto state = std::string(magic_enum::enum_name(state_c->current));
    ImGui::Text("State: %s", state.c_str());

    auto& actions_c = r.get_or_emplace<UnitCompletedActions>(e);
    const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), state_c->current);
    const bool action_available = it == actions_c.actions.end();

    if (state_c->current == UnitAction::MOVE) {

      // Debug from the current unit position, to the mouse position
      const auto map_e = get_first<MapComponent>(r);
      const auto& map_c = r.get<MapComponent>(map_e);
      const auto src_wp = get_position(r, e);
      const auto src_gp = engine::grid::worldspace_to_grid_space(src_wp, map_c.tilesize);
      const auto dst_gp = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
      const auto path = generate_direct_with_diagonals(r, src_gp, dst_gp);

      // Debug the active selection
      static EntityPool debug_path;
      debug_path.update(r, int(path.size()));
      for (size_t i = 0; i < path.size(); i++) {
        auto debug_e = debug_path.instances[i];
        auto pos = engine::grid::grid_space_to_world_space_center(path[i], map_c.tilesize);
        set_position(r, debug_e, pos);
        set_size(r, debug_e, { 8, 8 });
      }

      if (request_action) {
        GeneratedPathComponent path_c;
        path_c.path = path;
        path_c.path_cleared.resize(path.size(), false);
        path_c.src_pos = src_wp;
        path_c.dst_pos = mouse_pos;
        r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
      }
    }

    if (state_c->current == UnitAction::SHOOT) {
      //

      if (request_action) {
        SDL_Log("Wants to shoot...");
        //
      }
    }

    if (state_c->current == UnitAction::USE_ITEM) {
      //
      auto& actions_c = r.get_or_emplace<UnitCompletedActions>(e);
      const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), state_c->current);
      if (it == actions_c.actions.end()) {
        actions_c.actions.push_back(state_c->current);
        SDL_Log("wants to use item...");
      } else {
        SDL_Log("use action already taken... should end your turn");
      }
    }
  }

  ImGui::End();
}

} // namespace game2d