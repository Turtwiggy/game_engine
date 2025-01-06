#include "ui_action_bar_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/map/components.hpp"
#include "engine/map/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/actor_door/door_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_components.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_helpers.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include "modules/system_combat_bleed/combat_bleed_components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_move_player_on_map/move_player_on_map_components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_action_bar/ui_action_bar_components.hpp"
#include "modules/ui_combat_designer/ui_combat_designer_helpers.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <imgui.h>

namespace game2d {

const std::string invalid_move_icon = "TEXT_mul";
const std::string move_icon = "ARROW_UP";
const std::string atk_icon = "TEXT_mul";
const int debug_tilesize = 16;

bool
any_unit_is_moving(entt::registry& r)
{
  bool moving = false;

  for (const auto& [e, name_c] : r.view<const NameComponent>().each()) {
    const auto has_req = r.try_get<RequestMove>(e) != nullptr;
    const auto has_lerp = r.try_get<LerpToFixedTarget>(e) != nullptr;
    const auto& path_c = r.try_get<GeneratedPathComponent>(e);
    moving |= (has_lerp || has_req || path_c);
  }

  return moving;
};

bool
player_has_requested_action(entt::registry& r, entt::entity e)
{
  bool request_action = false;
  if (auto* inp_c = r.try_get<InputComponent>(e)) {
    request_action |= inp_c->shoot;

    // note: if two fixed_update() occur before

    // update to eat the shoot() event to prevent multiple requests
    // this is bad if any other system wants to use the shoot() action
    inp_c->shoot = false;
  }

  return request_action;
};

void
update_remove_path_when_at_destination(entt::registry& r, entt::entity e)
{
  GET_FIRST_OR_RETURN(MapComponent, r, map_e, map_c);

  const auto& path_view = r.view<const GeneratedPathComponent>();
  for (const auto& [e, generated_path_c] : path_view.each()) {

    if (generated_path_c.path.size() == 0)
      r.remove<GeneratedPathComponent>(e);

    const auto dst_gp = generated_path_c.path[generated_path_c.path.size() - 1];
    const auto cur_gp = get_grid_position(r, e);

    if (dst_gp != cur_gp)
      continue;

    const auto dst_idx = engine::grid::grid_position_to_index(dst_gp, map_c.xmax);
    move_entity_on_map(r, e, dst_idx);
    set_position_grid(r, e, dst_gp);

    if (auto* brain_c = r.try_get<DefaultBrainComponent>(e))
      brain_c->brain_fsm = BRAIN_STATE::IDLE;

    SDL_Log("Removing generated path..");
    r.remove<GeneratedPathComponent>(e);
  }
};

bool
action_available(const CompletedActions& actions_c, const std::string action)
{
  const auto it = std::find(actions_c.actions.begin(), actions_c.actions.end(), action);
  return it == actions_c.actions.end();
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
push_button_available_colours()
{
  // Active state: Green
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.33f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.33f, 0.8f, 0.8f));
};

void
push_button_unavailable_colours()
{
  // Inactive state: red
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
};

void
action_button(entt::registry& r,
              entt::entity e,
              std::string action,
              std::string label,
              const bool enabled_cond,
              const bool action_cond)
{
  const auto& team_c = r.get<const TeamComponent>(e);
  const auto& actions_c = r.get_or_emplace<CompletedActions>(e);

  const auto player_turn = team_c.team == AvailableTeams::player;

  const bool free_to_act = action_available(actions_c, action) && player_turn && enabled_cond;
  const ImVec2 size = { 64 * 16 / 9.0f, 32 };

  if (!free_to_act) {
    push_button_unavailable_colours();
    ImGui::BeginDisabled(true);

    if (ImGui::SameLine(); ImGui::Button(label.c_str(), size) || action_cond)
      r.emplace_or_replace<UIActionState>(e, action);

    ImGui::EndDisabled();
  }

  if (free_to_act) {
    push_button_available_colours();

    if (ImGui::SameLine(); ImGui::Button(label.c_str(), size) || action_cond)
      r.emplace_or_replace<UIActionState>(e, action);
  }

  ImGui::PopStyleColor(3);
};

void
display_actions_for_item(entt::registry& r, entt::entity e, entt::entity slot_e)
{
  const auto& slot_c = r.get<const InventorySlotComponent>(slot_e);
  if (slot_c.item_e == entt::null)
    return;
  const auto& in_c = get_first_component<SINGLE_InputComponent>(r);
  const bool moving = any_unit_is_moving(r);

  const auto item_eid = static_cast<uint32_t>(slot_c.item_e);
  ImGui::PushID(item_eid);

  const auto& item_ui_c = r.get<const UI_ItemComponent>(slot_c.item_e);
  // ImGui::Text("%s", item_ui_c.display_name.c_str());
  // ImGui::Text("%s", item_ui_c.display_desc.c_str());

  // List the actions that the equipped item can take.
  const auto& item_c = r.get<Item>(slot_c.item_e);

  if (item_c.combat.has_value())
    action_button(r, e, "attack", "(2) Attack", !moving, get_key_down(in_c, SDL_SCANCODE_2) && !moving);

  if (item_c.use.has_value())
    if (item_c.use->name == "heal")
      action_button(r, e, "heal", "(3) Heal", !moving, get_key_down(in_c, SDL_SCANCODE_3) && !moving);

  ImGui::PopID();
};

void
update_request_move_action(entt::registry& r, entt::entity e, const glm::ivec2 mouse_pos, bool request_action)
{
  GET_FIRST_OR_RETURN(MapComponent, r, map_e, map_c);

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

  // limit: if the last tile(s) are pathfinding cost -1, dont move to them
  std::vector<glm::ivec2> removed_tiles;
  if (path.size() >= 2) {
    for (int i = path.size(); i > 0; i--) {
      int cost = get_cost_at_gridpos(r, path[i - 1], map_c);
      if (cost != -1)
        break; // stop iterating backwards
      auto it = path.erase(path.begin() + i - 1);
      removed_tiles.push_back(*it);
    }
  }

  // Display the removed tiles
  for (const glm::ivec2& t : removed_tiles) {
    Sprite s;
    s.sprite = invalid_move_icon;
    s.pos = engine::grid::grid_space_to_world_space_center(t, map_c.tilesize);
    s.size = { debug_tilesize, debug_tilesize };
    s.z_idx = ZLayer::PLAYER_GUN_ABOVE_PLAYER;
    s.col = { 1.0f, 0.0f, 0.0f, 1.0f };
    draw_sprite(r, s);
  }

  // Display the active path
  if (path.size() > 0) {
    // note: -1 so not the tile you're standing on
    for (size_t i = 1; i < path.size(); i++) {
      Sprite s;
      s.sprite = move_icon;
      s.pos = engine::grid::grid_space_to_world_space_center(path[i], map_c.tilesize);
      s.size = { debug_tilesize, debug_tilesize };
      s.z_idx = ZLayer::PLAYER_GUN_ABOVE_PLAYER;
      s.col = { 0.0f, 1.0f, 0.0f, 1.0f };
      draw_sprite(r, s);
    }
  }

  if (request_action && path.size() > 0) {
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

  //
};

void
update_request_heal_action(entt::registry& r, entt::entity e)
{
  const auto& body_c = r.get<const DefaultBody>(e);

  RequestHeal req_heal;
  int heal = 0;

  // note: iterating heal across all items seems wrong
  for (const auto slot_e : body_c.body) {
    const auto& slot_c = r.get<const InventorySlotComponent>(slot_e);
    if (slot_c.item_e == entt::null)
      return;
    const auto& item_c = r.get<Item>(slot_c.item_e);
    heal += item_c.use->amount.value();
  }

  req_heal.amount = heal;
  r.emplace_or_replace<RequestHeal>(e, req_heal);
};

void
update_request_combat_action(entt::registry& r, entt::entity e, const glm::ivec2 mouse_pos, bool request_action)
{
  GET_FIRST_OR_RETURN(MapComponent, r, map_e, map_c);

  // The generated damage tiles that the weapon can use
  auto* tiles_c = r.try_get<TilesComponent>(e);
  if (!tiles_c)
    return;

  // if you're hovvering the damage tiles,
  // show as green, and if you click it while hovering, take the action
  const auto mouse_gp = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
  const bool hovering = std::find(tiles_c->tiles.begin(), tiles_c->tiles.end(), mouse_gp) != tiles_c->tiles.end();

  for (size_t i = 0; i < tiles_c->tiles.size(); i++) {
    const auto tile_gp = tiles_c->tiles[i];
    const auto tile_wsp = engine::grid::grid_space_to_world_space_center(tile_gp, map_c.tilesize);

    auto col = engine::SRGBColour{ 1.0f, 0.0f, 0.0f, 1.0f };
    if (hovering)
      col = engine::SRGBColour{ 0.0f, 1.0f, 0.0f, 1.0f };

    Sprite s;
    s.sprite = atk_icon;
    s.pos = tile_wsp;
    s.size = { debug_tilesize, debug_tilesize };
    s.z_idx = ZLayer::PLAYER_GUN_ABOVE_PLAYER;
    s.col = col;
    draw_sprite(r, s);
  }

  if (request_action && hovering)
    r.emplace_or_replace<RequestAttack>(e);

  if (request_action && !hovering)
    clear_actions(r, e);

  //
};

const auto process_actions = []<typename T>(entt::registry& r,
                                            entt::entity e,
                                            const T& t,
                                            const std::string& action,
                                            const std::function<void(entt::entity, T&)>& callback) {
  auto& actions_c = r.get_or_emplace<CompletedActions>(e);
  const auto view = r.view<T>();
  for (const auto& [req_e, req_c] : view.each()) {

    // clear the ui
    if (r.try_get<UIActionState>(req_e))
      r.remove<UIActionState>(req_e);

    // limit: only take action once
    if (!action_available(actions_c, action)) {
      SDL_Log("Already taken %s action this turn.", action.c_str());
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
  }

  // processed all requests
  r.remove<T>(view.begin(), view.end());
};

void
do_damage_action(entt::registry& r, const entt::entity e)
{
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  std::set<entt::entity> targets;

  // Get targets from RequestAttack (Ai sets targets in this Request)
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

  SDL_Log("Dealing damage...");

  // Get weapon info...
  const auto item_e = get_equipped_gun(r, e);
  const int dmg = get_damage_for_equipped_item(r, e);
  const DamageType dmg_type = DamageType::PHYSICAL;
  std::vector<Trait> weapon_traits;
  if (item_e != entt::null && r.get<Item>(item_e).traits.has_value())
    weapon_traits = r.get<Item>(item_e).traits.value();

  // attack targets, not tiles
  for (const auto map_e : targets) {
    DamageEvent evt;
    evt.from = e;
    evt.to = map_e;
    evt.type = dmg_type;
    evt.amount = dmg;
    evt.traits = weapon_traits;

    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }
};

void
update_process_move_request(entt::registry& r, entt::entity e)
{
  const std::function<void(entt::entity, RequestMove&)> move_callback = [&r](entt::entity e, const RequestMove& req_c) {
    //
    // Action is available
    r.emplace_or_replace<GeneratedPathComponent>(e, req_c.path_c);
  };
  process_actions(r, e, RequestMove(), "move", move_callback);
};

void
update_process_heal_request(entt::registry& r, entt::entity e)
{
  const std::function<void(entt::entity, RequestHeal&)> callback = [&r](entt::entity e, const RequestHeal& req_c) {
    //
    // Action is available
    auto& hp_c = r.get<HealthComponent>(e);
    hp_c.hp += req_c.amount;
    hp_c.hp = glm::min(hp_c.hp, hp_c.max_hp);
    SDL_Log("Healing for %i", req_c.amount);

    // Note: healing cures bleed
    if (const auto* bleed_c = r.try_get<BleedComponent>(e))
      r.remove<BleedComponent>(e);

    // consume item...!
    // if (item_c.use->uses.has_value() && item_c.use->uses.value() == 1) {
    // }
  };
  process_actions(r, e, RequestHeal(), "heal", callback);
};

void
update_process_combat_request(entt::registry& r, entt::entity e)
{
  const std::function<void(entt::entity, RequestAttack&)> shoot_callback = [&r](entt::entity e, const RequestAttack& req_c) {
    //
    // Action is available
    do_damage_action(r, e);

    // The damage action could be animated, but for now,
    // immediately set back to idle as no anim implemented
    if (auto* brain_c = r.try_get<DefaultBrainComponent>(e))
      brain_c->brain_fsm = BRAIN_STATE::IDLE;
  };
  process_actions(r, e, RequestAttack(), "shoot", shoot_callback);
};

void
update_process_end_turn_request(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_Initiative, r, init_e, init_c);
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
};

} // namespace game2d