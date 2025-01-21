#include "ui_action_bar_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "modules/combat/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_move_player_on_map/move_player_on_map_components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_action_bar/ui_action_bar_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "ui_action_bar_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_scancode.h>
#include <imgui.h>
#include <magic_enum.hpp>

#include <memory>

namespace game2d {

void
ai_tick(entt::registry& r, entt::entity e)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  const auto name = r.get<NameComponent>(e).name;
  auto& brain_c = r.get<DefaultBrainComponent>(e);
  if (brain_c.brain_fsm == BRAIN_STATE::IDLE) {
    SDL_Log("AI: IDLE => REASONING");
    brain_c.brain_fsm = BRAIN_STATE::REASONING;

    // Given all the actions the ai can take, choose one
    SDL_Log("Ai (%s) deciding what to do...", name.c_str());
    const std::optional<std::shared_ptr<Action>> action = Reasoner::Evaluate(r, e, brain_c);

    if (action.has_value()) {
      SDL_Log("AI has chosen to... %s", action.value()->GetClassName().c_str());

      // AI: REASONING => MOVE
      if (std::dynamic_pointer_cast<MoveAction>(action.value())) {
        // ai: use path ai has chosen
        auto path = r.get<MoveConsiderationData>(e).final_path;

        // limit: limit path based on movement.
        if (auto* limit_c = r.try_get<LimitMovementComponent>(e)) {
          const int n = limit_c->path_size;
          path = { path.begin(), path.begin() + std::min(n + 1, (int)path.size()) };
        };

        GeneratedPathComponent path_c;
        path_c.path = path;
        path_c.path_cleared.resize(path.size(), false);
        path_c.src_pos = get_position(r, e);
        path_c.dst_pos = engine::grid::grid_space_to_world_space_center(path[path.size() - 1], map_c.tilesize);

        RequestMove req_c;
        req_c.path_c = path_c;
        r.emplace<RequestMove>(e, req_c);

        brain_c.brain_fsm = BRAIN_STATE::MOVE;
      }

      // AI: REASONING => ATTACK
      if (std::dynamic_pointer_cast<AttackAction>(action.value())) {
        const auto& data_c = r.get<AttackConsiderationData>(e);
        r.emplace<RequestAttack>(e, data_c.targets);

        brain_c.brain_fsm = BRAIN_STATE::ANIMATE;
      }
    }
  }

  // State: REASONING => IDLE
  // If we've ended in a reasoning state,
  // assume that ai didnt choose an action,
  // and end the turn.
  if (brain_c.brain_fsm == BRAIN_STATE::REASONING) {
    const auto brain_state_str = engine::convert_enum_to_string(brain_c.brain_fsm);
    brain_c.brain_fsm = BRAIN_STATE::IDLE;

    SDL_Log("AI requesting end turn...");
    r.emplace_or_replace<RequestEndTurn>(e);
  }
};

void
update_ui_action_bar_system(entt::registry& r, const glm::ivec2 mouse_pos)
{
  GET_FIRST_OR_RETURN(SINGLE_Initiative, r, init_e, init_c);
  GET_FIRST_OR_RETURN(MapComponent, r, map_e, map_c);
  const auto& in_c = get_first_component<SINGLE_InputComponent>(r);
  const auto& ri_c = get_first_component<SINGLE_RendererInfo>(r);

  // no units with initiative
  if (init_c.order.size() == 0)
    return;

  const auto& team_view = r.view<const TeamComponent>();
  const auto players = filter_view(team_view, [&](entt::entity e) {
    const auto& team_c = team_view.get<TeamComponent>(e);
    return team_c.team == AvailableTeams::player;
  });

  if (players.size() == 0)
    return; // no players

  // This is the unit currnetly "active" in the initiative order.
  const auto e = init_c.order[0];
  const auto eid = static_cast<uint32_t>(e);
  const auto& body_c = r.get<const DefaultBody>(e);
  const auto& team_c = r.get<const TeamComponent>(e);
  const auto player_turn = team_c.team == AvailableTeams::player;
  const auto enemy_turn = team_c.team == AvailableTeams::enemy;

  bool request_action = false;
  request_action |= player_has_requested_action(r, e);
  request_action &= (ri_c.viewport_hovered); // no ui

  // position
  const ImVec2 viewport_pos = { (float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y };
  const ImVec2 viewport_size_half = ImVec2(ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f);
  const float center_x = viewport_pos.x + viewport_size_half.x;
  const float bottom_y = viewport_pos.y + ri_c.viewport_size_render_at.y - 50.0f;
  const auto pos = ImVec2(center_x, bottom_y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  ImGui::Begin("Action Bar", NULL, flags);
  ImGui::PushID(eid);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
  {
    // Display actions
    // UI state: no state => selected action
    {
      const bool moving = any_unit_is_moving(r);
      action_button(r, e, action_move_key, "(1) Move", !moving, get_key_down(in_c, SDL_SCANCODE_1) && !moving);

      for (const auto slot_e : body_c.body)
        display_actions_for_item(r, e, slot_e);

      action_button(r, e, action_end_turn_key, "(E)nd", !moving, get_key_down(in_c, SDL_SCANCODE_E));

      if (ImGui::SameLine(); ImGui::Button("Clear")) {
        if (r.try_get<UIActionState>(e))
          r.remove<UIActionState>(e);
      }
    }

    //
    // If an action_button was selected,
    // The state will be stored in UIActionState
    // Here, process ui selected action => request action
    //
    if (auto* state_c = r.try_get<UIActionState>(e)) {
      const auto state = state_c->current;
      ImGui::Text("State: %s", state.c_str());

      // Adds the RequestMove component to e when a tile is selected
      if (state_c->current == action_move_key)
        update_request_move_action(r, e, mouse_pos, request_action);

      // If state was heal, immediately request to heal
      if (state_c->current == action_heal_key)
        update_request_heal_action(r, e);

      // Adds the RequestAttack component to e when a tile is selected
      if (state_c->current == action_attack_key)
        update_request_combat_action(r, e, mouse_pos, request_action);

      // If state was end turn, immediately request to end turn
      if (state_c->current == action_end_turn_key)
        r.emplace_or_replace<RequestEndTurn>(e);
    }
  }
  ImGui::PopStyleVar(1);
  ImGui::PopID();
  ImGui::End();

  if (enemy_turn)
    ai_tick(r, e);

  update_remove_path_when_at_destination(r, e);
  update_process_move_request(r, e);
  update_process_heal_request(r, e);
  update_process_combat_request(r, e);
  update_process_end_turn_request(r);
}

} // namespace game2d