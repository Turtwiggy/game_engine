#include "ui_action_bar_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/grid.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_door/door_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/animations/wiggle/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_show_tiles_in_range/show_tileS_in_range_helpers.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include "modules/system_combat_bleed/combat_bleed_components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_move_player_on_map/move_player_on_map_components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_action_bar/ui_action_bar_components.hpp"
#include "modules/ui_combat_designer/ui_combat_designer_helpers.hpp"
#include "ui_action_bar_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <SDL2/SDL_scancode.h>
#include <imgui.h>
#include <magic_enum.hpp>

#include <memory>

namespace game2d {

void
do_damage_action(entt::registry& r, entt::entity e)
{
  const auto& evts = get_first_component<SINGLE_Events>(r);

  std::set<entt::entity> targets;

  // Set targets for AI
  const auto* ai_targets = r.try_get<RequestAttack>(e);
  if (ai_targets && ai_targets->targets.size() > 0)
    targets = { ai_targets->targets.begin(), ai_targets->targets.end() };

  // Set targets via Tiles
  else if (const auto* tiles = r.try_get<TilesComponent>(e)) {
    for (const glm::ivec2& tile : tiles->tiles) {
      // damage all mobs (off map)
      const auto mobs = contains_mobs(r, tile);
      targets.insert(mobs.begin(), mobs.end());
    }
  }

  if (targets.size() == 0) {
    SDL_Log("Tried to attack but no targets...");
    return;
  }

  SDL_Log("Dealing damage to targets in RequestAttack...");

  // Get weapon info...
  const auto item_e = get_equipped_gun(r, e);
  const int dmg = get_damage_for_equipped_item(r, e);
  const DamageType dmg_type = DamageType::PHYSICAL;
  std::vector<Trait> weapon_traits;
  if (item_e != entt::null && r.get<Item>(item_e).traits.has_value())
    weapon_traits = r.get<Item>(item_e).traits.value();

  // attack these, not tiles
  for (const auto map_e : targets) {
    DamageEvent evt;
    evt.from = e;
    evt.to = map_e;
    evt.type = dmg_type;
    evt.amount = dmg;
    evt.traits = weapon_traits;

    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();
  }
};

bool
action_available(const CompletedActions& actions_c, ActionEnum action)
{
  const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), action);
  return it == actions_c.actions.end();
};

void
push_button_available_colours()
{
  // Active state: Green
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.9f, 0.1f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
};

void
push_button_unavailable_colours()
{
  // Inactive state: red
  ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
};

void
ai_tick(entt::registry& r, entt::entity e)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  const auto name = r.get<NameComponent>(e).first_name;
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

  // State: MOVE => IDLE
  if (brain_c.brain_fsm == BRAIN_STATE::MOVE) {
    const auto has_req = r.try_get<RequestMove>(e) != nullptr;
    const auto has_lerp = r.try_get<LerpToFixedTarget>(e) != nullptr;
    const auto& path_c = r.try_get<GeneratedPathComponent>(e);
    const bool moving = has_lerp || has_req;
    const bool arrived = at_destination(r, e);
    const bool just_finished_moving = path_c && !moving && arrived;

    if (just_finished_moving) {
      brain_c.brain_fsm = BRAIN_STATE::IDLE;

      if (path_c->path.size() > 0) {
        const auto dst_idx = engine::grid::worldspace_to_index(path_c->dst_pos, map_c.tilesize, map_c.xmax, map_c.ymax);
        move_entity_on_map(r, e, dst_idx);
        r.remove<GeneratedPathComponent>(e);
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
clear_actions(entt::registry& r, entt::entity e)
{
  // .. remove in progress action
  if (r.try_get<UIActionState>(e))
    r.remove<UIActionState>(e);

  if (r.try_get<GeneratedPathComponent>(e))
    r.remove<GeneratedPathComponent>(e);
};

void
update_ui_action_bar_system(entt::registry& r, const glm::ivec2 mouse_pos)
{
  const auto init_e = get_first<SINGLE_Initiative>(r);
  if (init_e == entt::null)
    return;
  const auto& init_c = r.get<SINGLE_Initiative>(init_e);
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);

  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = get_first_component<MapComponent>(r);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  const auto end_turn_key = SDL_SCANCODE_E;

  if (init_c.order.size() == 0)
    return; // no units with initiative

  int n_players = 0;
  for (const auto& [e, team_c] : r.view<TeamComponent>().each())
    if (team_c.team == AvailableTeams::player)
      n_players++;
  if (n_players == 0)
    return; // no players

  // it's the first unit's turn...
  const auto e = init_c.order[0];

  const bool player_turn = r.get<TeamComponent>(e).team == AvailableTeams::player;
  const bool enemy_turn = r.get<TeamComponent>(e).team == AvailableTeams::enemy;

  bool request_action = false;
  if (auto* inp_c = r.try_get<InputComponent>(e))
    request_action |= inp_c->shoot;
  request_action &= (ri.viewport_hovered); // no ui
  if (request_action)
    SDL_Log("requesting action...");

  static EntityPool debug_path;

  // clear ui visuals if no action selected
  if (r.try_get<UIActionState>(e) == nullptr)
    debug_path.update(r, 0);

  // position
  const ImVec2 viewport_pos = { (float)ri.viewport_pos.x, (float)ri.viewport_pos.y };
  const ImVec2 viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const float center_x = viewport_pos.x + viewport_size_half.x;
  const float bottom_y = viewport_pos.y + ri.viewport_size_current.y - 50.0f;
  const auto pos = ImVec2(center_x, bottom_y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGuiWindowFlags flags = 0;
  // flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  // Is the current entity moving?
  const auto has_req = r.try_get<RequestMove>(e) != nullptr;
  const auto has_lerp = r.try_get<LerpToFixedTarget>(e) != nullptr;
  const auto& path_c = r.try_get<GeneratedPathComponent>(e);
  const auto has_path = path_c != nullptr;
  const bool moving = has_lerp || has_req;
  const bool arrived = at_destination(r, e);
  const bool just_finished_moving = path_c && !moving && arrived;

  ImGui::Begin("Action Bar", NULL, flags);
  {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

    const auto button_enabled = [&r, e, &player_turn](const ActionEnum& a,
                                                      std::string label,
                                                      const bool additional_enabled_cond = true,
                                                      const bool additional_action_cond = false) {
      const auto& actions_c = r.get_or_emplace<CompletedActions>(e);
      const bool free_to_act = action_available(actions_c, a) && player_turn && additional_enabled_cond;
      const ImVec2 size = { 64 * 16 / 9.0f, 32 };

      if (!free_to_act) {
        push_button_unavailable_colours();
        ImGui::BeginDisabled(true);

        if (ImGui::SameLine(); ImGui::Button(label.c_str(), size) || additional_action_cond)
          r.emplace_or_replace<UIActionState>(e, a);

        ImGui::EndDisabled();
      }

      if (free_to_act) {
        push_button_available_colours();

        if (ImGui::SameLine(); ImGui::Button(label.c_str(), size) || additional_action_cond)
          r.emplace_or_replace<UIActionState>(e, a);
      }

      ImGui::PopStyleColor(3);
    };

    button_enabled(ActionEnum::MOVE, "(1) Move", true, get_key_down(input_c, SDL_SCANCODE_1) && !moving);
    button_enabled(ActionEnum::SHOOT, "(2) Attack", true, get_key_down(input_c, SDL_SCANCODE_2) && !moving);
    button_enabled(ActionEnum::USE_ITEM, "(3) Use (Heal)", true, get_key_down(input_c, SDL_SCANCODE_3) && !moving);

    const bool allowed_to_end = !any_unit_is_moving(r);
    button_enabled(ActionEnum::END_TURN, "(E)nd", allowed_to_end, get_key_down(input_c, SDL_SCANCODE_E) && !moving);

    if (auto* state_c = r.try_get<UIActionState>(e)) {
      const auto state = std::string(magic_enum::enum_name(state_c->current));
      ImGui::Text("State: %s", state.c_str());

      if (state_c->current == ActionEnum::MOVE) {

        // Debug from the current unit position, to the mouse position
        const auto map_e = get_first<MapComponent>(r);
        const auto& map_c = r.get<MapComponent>(map_e);
        const auto src_wp = get_position(r, e);
        const auto dst_wp = mouse_pos;
        const auto src_gp = engine::grid::worldspace_to_grid_space(src_wp, map_c.tilesize);
        const auto dst_gp = engine::grid::worldspace_to_grid_space(dst_wp, map_c.tilesize);
        auto path = generate_direct_with_diagonals(r, src_gp, dst_gp);

        // limit: limit path based on movement.
        if (auto* limit_c = r.try_get<LimitMovementComponent>(e)) {
          const int n = limit_c->path_size;
          path = { path.begin(), path.begin() + std::min(n + 1, (int)path.size()) };
        };

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
          path_c.dst_pos = engine::grid::grid_space_to_world_space_center(path[path.size() - 1], map_c.tilesize);

          RequestMove req_c;
          req_c.path_c = path_c;
          r.emplace<RequestMove>(e, req_c);
        }
      }

      if (state_c->current == ActionEnum::SHOOT) {

        // show damage tiles
        // wiggle the attack icon

        if (auto* tiles_c = r.try_get<TilesComponent>(e)) {
          debug_path.update(r, int(tiles_c->tiles.size()));

          // if you're hovvering the damage tiles,
          // show as green, and if you click it while hovering, take the action
          const auto mouse_gp = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
          const bool hovering = std::find(tiles_c->tiles.begin(), tiles_c->tiles.end(), mouse_gp) != tiles_c->tiles.end();

          for (size_t i = 0; i < tiles_c->tiles.size(); i++) {
            const auto tile_gp = tiles_c->tiles[i];
            const auto tile_wsp = engine::grid::grid_space_to_world_space_center(tile_gp, map_c.tilesize);
            const auto debug_e = debug_path.instances[i];

            set_position(r, debug_e, tile_wsp);
            set_sprite(r, debug_e, "CROSSHAIR_10");
            set_size(r, debug_e, { 32, 32 });
            set_z_index(r, debug_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);

            if (hovering)
              set_colour(r, debug_e, { 0.0f, 1.0f, 0.0f, 1.0f });
            else
              set_colour(r, debug_e, { 1.0f, 0.0f, 0.0f, 1.0f });

            // const auto* wiggle_c = r.try_get<WiggleUpAndDown>(debug_e);
            // if (!wiggle_c) {
            //   WiggleUpAndDown wiggle_c;
            //   wiggle_c.base_position = tile_wsp;
            //   wiggle_c.amplitude = 1.0;
            //   r.emplace<WiggleUpAndDown>(debug_e, wiggle_c);
            // }
          }

          if (request_action && hovering)
            r.emplace_or_replace<RequestAttack>(e);

          if (request_action && !hovering)
            clear_actions(r, e);
        }
      }

      if (state_c->current == ActionEnum::USE_ITEM)
        r.emplace_or_replace<RequestItem>(e);

      if (state_c->current == ActionEnum::END_TURN)
        r.emplace_or_replace<RequestEndTurn>(e);
    }
  }

  ImGui::PopStyleVar(1);
  ImGui::End();

  if (enemy_turn)
    ai_tick(r, e);

  // monitor when the entity has stopped moving
  //
  if (just_finished_moving) {
    const auto dst_idx = engine::grid::worldspace_to_index(path_c->dst_pos, map_c.tilesize, map_c.xmax, map_c.ymax);
    move_entity_on_map(r, e, dst_idx);

    r.remove<GeneratedPathComponent>(e);

    if (auto* brain_c = r.try_get<DefaultBrainComponent>(e))
      brain_c->brain_fsm = BRAIN_STATE::IDLE;

    if (auto* action_c = r.try_get<UIActionState>(e))
      if (action_c->current == ActionEnum::MOVE)
        r.remove<UIActionState>(e);
  }

  auto& actions_c = r.get_or_emplace<CompletedActions>(e);

  const auto process_actions = [&r, &actions_c]<typename T>(const T& t,
                                                            const ActionEnum& action,
                                                            const std::function<void(entt::entity, T&)>& callback) {
    const auto view = r.view<T>();

    for (const auto& [req_e, req_c] : view.each()) {

      // limit: only take action once
      if (!action_available(actions_c, action)) {
        const auto action_str = std::string(magic_enum::enum_name(action));
        SDL_Log("Already taken %s action this turn.", action_str.c_str());
        r.remove<T>(req_e);

        if (auto* brain_c = r.try_get<DefaultBrainComponent>(req_e)) {
          SDL_Log("AI likely requested repeat action... ending their turn");
          brain_c->brain_fsm = BRAIN_STATE::REASONING;
        }

        continue;
      }

      // Action is available!
      callback(req_e, req_c);

      // Set the action as completed, and remove the request
      actions_c.actions.push_back(action);

      // clear the ui
      if (r.try_get<UIActionState>(req_e))
        r.remove<UIActionState>(req_e);
    }

    // processed all requests
    r.remove<T>(view.begin(), view.end());
  };

  const std::function<void(entt::entity, RequestMove&)> move_callback = [&r](entt::entity e, const RequestMove& req_c) {
    //
    // Action is available
    r.emplace_or_replace<GeneratedPathComponent>(e, req_c.path_c);
  };
  process_actions(RequestMove(), ActionEnum::MOVE, move_callback);

  const std::function<void(entt::entity, RequestAttack&)> shoot_callback = [&r](entt::entity e, const RequestAttack& req_c) {
    //
    // Action is available
    do_damage_action(r, e);

    // The damage action could be animated, but for now,
    // immediately set back to idle as no anim implemented
    if (auto* brain_c = r.try_get<DefaultBrainComponent>(e))
      brain_c->brain_fsm = BRAIN_STATE::IDLE;
  };
  process_actions(RequestAttack(), ActionEnum::SHOOT, shoot_callback);

  const std::function<void(entt::entity, RequestItem&)> item_callback = [&r](entt::entity e, const RequestItem& req_c) {
    //
    // hack: heal. this should actually be based on items, not just always be a heal
    auto& hp = r.get<HealthComponent>(e);
    hp.hp += 10;
    hp.hp = glm::min(hp.hp, hp.max_hp);
    if (const auto* bleed_c = r.try_get<BleedComponent>(e))
      r.remove<BleedComponent>(e); // fix bleed
  };
  process_actions(RequestItem(), ActionEnum::USE_ITEM, item_callback);

  // end turn impl
  auto& evts = get_first_component<SINGLE_Events>(r);
  const auto view_req = r.view<RequestEndTurn>();
  for (const auto& [req_e, req_c] : view_req.each()) {
    SDL_Log("~~~~~~~~~ ending turn ~~~~~~~~~");

    clear_actions(r, req_e);

    // .. remove completed actions
    if (r.try_get<CompletedActions>(req_e))
      r.remove<CompletedActions>(req_e);

    // Set the initiative of the first unit to one higher than the last unit
    if (init_c.order.size() > 1) {
      const auto last_e = init_c.order[init_c.order.size() - 1];
      const auto& last_c = r.get<InitiativeComponent>(last_e);
      auto& first_c = r.get<InitiativeComponent>(req_e);

      // This line here is weird. It probably shouldnt be +1,
      // but should be relative to the units own agility or dexterity.
      // This means that faster units would take turns more frequently.
      first_c.initiative = last_c.initiative + 1;

      const auto next_e = init_c.order[1];
      activate_unit(r, next_e);
    }

    // Fire end turn event for this entity
    EndTurnEvent evt;
    evt.e = req_e;
    evts.dispatcher->trigger(evt);
    evts.dispatcher->update();
  }
  r.remove<RequestEndTurn>(view_req.begin(), view_req.end());
}

} // namespace game2d