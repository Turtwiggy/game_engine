#include "system.hpp"

#include "components.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_turnbased/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
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

void
change_cursor(entt::registry& r, const CursorType& type)
{
  const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& map = get_first_component<MapComponent>(r);
  const auto& e = info.action_cursor;
  const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;

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

  static Actions action = Actions::MOVE;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_1))
    action = Actions::MOVE;
  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_2))
    action = Actions::ATTACK;

  int mouse_grid_increments = 1;
  if (action == Actions::ATTACK)
    mouse_grid_increments = 1;
  if (action == Actions::MOVE)
    mouse_grid_increments = map.tilesize;

  const bool rmb_click = get_mouse_rmb_press();
  const int grid_snap_size = mouse_grid_increments; // note: this is not the map.tilesize,

  const auto mouse_gridspace = engine::grid::world_space_to_grid_space(input_mouse_pos, grid_snap_size);
  auto mouse_pos = engine::grid::grid_space_to_world_space(mouse_gridspace, grid_snap_size);
  if (action == Actions::MOVE)
    mouse_pos += glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f }; // center it

  // change cursor icon
  if (action == Actions::MOVE)
    change_cursor(r, CursorType::MOVE);
  if (action == Actions::ATTACK)
    change_cursor(r, CursorType::ATTACK);

  // Limit: the next bit should only happen if it's not the start of the game
  auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team == AvailableTeams::neutral)
    return;

  // Hack: display all units hp/defence in worldspace
  // const auto& view = r.view<HealthComponent, DefenceComponent, AABB>();
  // for (const auto& [e, hp, defence, aabb] : view.each()) {
  //   auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
  //   worldspace_ui.text = "HP:"s + std::to_string(hp.hp) + " DEF:" + std::to_string(defence.armour);
  //   worldspace_ui.offset.y = -aabb.size.y;
  // }

  const auto& selected_view = r.view<SelectedComponent, AABB>();
  int count = 0;
  for (const auto& [e, selected_c, aabb] : selected_view.each())
    count++;

  // stop showing movement path
  if (count == 0 || action != Actions::MOVE)
    state.show_selected_player_path.update(r, 0);

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
  if (state.team != AvailableTeams::player)
    return;

  for (const auto& [e, selected_c, aabb] : selected_view.each()) {

    // UX: show the path the unit would take
    if (action == Actions::MOVE) {
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

    auto& actions = r.get_or_emplace<ActionState>(e);

    // already has a path that you've not yet arrived at
    bool able_to_move = !has_destination(r, e) || at_destination(r, e);
    able_to_move &= !destination_is_blocked(r, mouse_pos);

    // move mode
    if (action == Actions::MOVE && able_to_move && actions.actions_available > 0 && rmb_click) {
      const auto limit = r.get<MoveLimitComponent>(e).amount;
      const auto path = generate_path(r, e, mouse_pos, limit);
      update_entity_path(r, e, path);

      actions.actions_available--;
      actions.actions_completed++;
    }

    // shoot mode
    if (action == Actions::ATTACK && actions.actions_available > 0 && rmb_click) {
      r.emplace_or_replace<WantsToShoot>(e);

      actions.actions_available--;
      actions.actions_completed++;
    }

    if (actions.actions_available == 0)
      create_empty<RequestToCompleteTurn>(r, RequestToCompleteTurn{ AvailableTeams::player });
  }
}

} // namespace game2d