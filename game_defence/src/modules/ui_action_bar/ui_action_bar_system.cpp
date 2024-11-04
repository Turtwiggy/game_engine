#include "ui_action_bar_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_door/door_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/ui_action_bar/ui_action_bar_components.hpp"
#include "modules/ui_combat_designer/ui_combat_designer_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_scancode.h>
#include <imgui.h>
#include <magic_enum.hpp>
#include <memory>

namespace game2d {

void
do_damage_action(entt::registry& r, entt::entity e)
{
  const auto* damage_tiles = r.try_get<TilesComponent>(e);
  if (!damage_tiles)
    return;
  const auto& evts = get_first_component<SINGLE_Events>(r);
  SDL_Log("Dealing damage in highlighted tiles...");

  for (const auto& tile : damage_tiles->tiles) {
    // damage all mobs (off map)
    std::vector<entt::entity> mobs = contains_mobs(r, tile);

    // damage all entities (on map)
    std::set<entt::entity> unique_mobs{ mobs.begin(), mobs.end() };
    // for (const auto map_e : map_c.map[idx])
    //   unique_mobs.emplace(map_e);

    for (const auto map_e : unique_mobs) {
      DamageEvent evt;
      evt.from = e;
      evt.to = map_e;

      // note: random damage, but this isn't correct
      static engine::RandomState rnd(0);
      evt.amount = engine::rand_det_s(rnd.rng, 0, 10);

      evts.dispatcher->trigger(evt);
      evts.dispatcher->update();
    }
  }
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

  const ImVec4 col_active = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const ImVec4 col_inactive = ImVec4(0.8f, 0.5f, 0.5f, 1.0f);

  if (init_c.order.size() == 0)
    return; // no units with initiative

  // it's the first unit's turn...
  const auto e = init_c.order[0];

  bool request_action = false;
  if (auto* inp_c = r.try_get<InputComponent>(e))
    request_action |= inp_c->shoot;
  request_action &= (ri.viewport_hovered); // no ui
  if (request_action)
    SDL_Log("requesting action...");

  const bool player_turn = r.get<TeamComponent>(e).team == AvailableTeams::player;
  const bool enemy_turn = r.get<TeamComponent>(e).team == AvailableTeams::enemy;

  static EntityPool debug_path;
  debug_path.update(r, 0);

  ImGui::Begin("Action Bar");
  {
    if (ImGui::Button("Clear") || get_mouse_rmb_press()) {
      if (r.try_get<UIActionState>(e))
        r.remove<UIActionState>(e);
      if (r.try_get<GeneratedPathComponent>(e))
        r.remove<GeneratedPathComponent>(e);
    }

    if (ImGui::SameLine(); ImGui::Button("Move"))
      r.emplace_or_replace<UIActionState>(e, ActionEnum::MOVE);

    if (ImGui::SameLine(); ImGui::Button("Shoot"))
      r.emplace_or_replace<UIActionState>(e, ActionEnum::SHOOT);

    if (ImGui::SameLine(); ImGui::Button("Use"))
      r.emplace_or_replace<UIActionState>(e, ActionEnum::USE_ITEM);

    const bool allowed_to_end = !has_destination(r, e);
    const ImVec4 col = allowed_to_end ? col_active : col_inactive;
    if (ImGui::SameLine(); (ImGui::ColorButton("End", col) && allowed_to_end))
      create_empty<RequestEndTurn>(r);

    if (auto* state_c = r.try_get<UIActionState>(e)) {
      const auto state = std::string(magic_enum::enum_name(state_c->current));
      ImGui::Text("State: %s", state.c_str());

      auto& actions_c = r.get_or_emplace<CompletedActions>(e);
      const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), state_c->current);
      const bool action_available = it == actions_c.actions.end();

      glm::vec2 dst_wp{ 0, 0 };
      if (player_turn)
        dst_wp = mouse_pos;

      if (state_c->current == ActionEnum::MOVE) {

        // Debug from the current unit position, to the mouse position
        const auto map_e = get_first<MapComponent>(r);
        const auto& map_c = r.get<MapComponent>(map_e);
        const auto src_wp = get_position(r, e);
        const auto src_gp = engine::grid::worldspace_to_grid_space(src_wp, map_c.tilesize);
        const auto dst_gp = engine::grid::worldspace_to_grid_space(dst_wp, map_c.tilesize);
        const auto path = generate_direct_with_diagonals(r, src_gp, dst_gp);

        // Debug the active selection
        debug_path.update(r, int(path.size()));
        for (size_t i = 0; i < path.size(); i++) {
          auto debug_e = debug_path.instances[i];
          auto pos = engine::grid::grid_space_to_world_space_center(path[i], map_c.tilesize);
          set_position(r, debug_e, pos);
          set_size(r, debug_e, { 8, 8 });
        }

        if (request_action) {
          SDL_Log("Requesting move...");

          GeneratedPathComponent path_c;
          path_c.path = path;
          path_c.path_cleared.resize(path.size(), false);
          path_c.src_pos = src_wp;
          path_c.dst_pos = mouse_pos;

          RequestMove req_c;
          req_c.path_c = path_c;
          r.emplace<RequestMove>(e, req_c);
        }
      }

      if (state_c->current == ActionEnum::SHOOT && request_action) {
        SDL_Log("Requesting shoot...");

        auto& actions_c = r.get_or_emplace<CompletedActions>(e);
        const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), state_c->current);
        if (it == actions_c.actions.end()) {
          r.emplace_or_replace<RequestAttack>(e);

          // uncomment this means one shoot per turn
          // actions_c.actions.push_back(state_c->current);
        }
      }

      if (state_c->current == ActionEnum::USE_ITEM) {
        auto& actions_c = r.get_or_emplace<CompletedActions>(e);
        const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), state_c->current);
        if (it == actions_c.actions.end()) {
          SDL_Log("TODO: wants to use item...");

          actions_c.actions.push_back(state_c->current); // done
        }
      }
    }
  }
  ImGui::End();

  if (enemy_turn) {
    auto& brain_c = r.get<DefaultBrainComponent>(e);
    if (brain_c.brain_fsm == BRAIN_STATE::IDLE) {
      SDL_Log("changing ai to reasoning...");
      brain_c.brain_fsm = BRAIN_STATE::REASONING;

      const auto action = Reasoner::Evaluate(r, e, brain_c);
      if (action.has_value()) {
        const std::string name = typeid(action.value()).name();
        SDL_Log("AI has chosen to... %s", name.c_str());

        if (std::dynamic_pointer_cast<MoveAction>(action.value())) {
          //
          // TODO: ai: Generate a path
          //
          std::vector<glm::ivec2> path;

          GeneratedPathComponent path_c;
          path_c.path = path;
          path_c.path_cleared.resize(path.size(), false);
          path_c.src_pos = get_position(r, e);
          path_c.dst_pos = mouse_pos;

          RequestMove req_c;
          req_c.path_c = path_c;
          r.emplace<RequestMove>(e, req_c);
        }

        if (std::dynamic_pointer_cast<AttackAction>(action.value())) {
          //
          // TODO: ai: Get all targets....
          //
          r.emplace<RequestAttack>(e);
        }
      }

      // reasoning... i.e. done
      //
      if (brain_c.brain_fsm == BRAIN_STATE::REASONING) {
        const auto brain_state_str = engine::convert_enum_to_string(brain_c.brain_fsm);
        SDL_Log("BRAIN_STATE::%s", brain_state_str.c_str());
        brain_c.brain_fsm = BRAIN_STATE::IDLE;

        create_empty<RequestEndTurn>(r); // assume done...
      }
    }
  }

  // request move impl
  for (const auto& [e, req_c] : r.view<RequestMove>().each()) {
    r.emplace_or_replace<GeneratedPathComponent>(e, req_c.path_c);
    r.remove<RequestMove>(e);
  }

  // request shoot impl
  for (const auto& [e, req_c] : r.view<RequestAttack>().each()) {
    do_damage_action(r, e);
    r.remove<RequestAttack>(e);
  }

  // end turn impl
  if (get_first<RequestEndTurn>(r) != entt::null) {
    SDL_Log("ending turn...");

    // .. remove in progress action
    if (r.try_get<UIActionState>(e))
      r.remove<UIActionState>(e);

    // .. remove completed actions
    if (r.try_get<CompletedActions>(e))
      r.remove<CompletedActions>(e);

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

    // process requests
    const auto& view = r.view<RequestEndTurn>();
    r.destroy(view.begin(), view.end());
  }
}

} // namespace game2d