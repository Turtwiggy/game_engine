#include "system.hpp"

#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/system_turnbased_enemy/helpers.hpp"
#include "modules/ui_lootbag/components.hpp"
#include "modules/ux_hoverable/components.hpp"

#include <fmt/core.h>

#include <cmath>

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
// change_cursor(entt::registry& r, const CursorType& type)
// {
//   const auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
//   const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
//   const auto& map = get_first_component<MapComponent>(r);
//   const auto& e = info.action_cursor;
//   const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;

//   if (type == CursorType::EMPTY) {
//     set_size(r, e, { 0, 0 });
//     set_sprite_custom(r, e, "EMPTY"s, tex_unit_kenny);
//   } else if (type == CursorType::MOVE) {
//     set_size(r, e, { map.tilesize, map.tilesize });
//     set_sprite_custom(r, e, "CURSOR_1"s, tex_unit_kenny);
//   } else if (type == CursorType::ATTACK) {
//     set_size(r, e, { 16, 16 });
//     set_sprite_custom(r, e, "CURSOR_ATTACK_2"s, tex_unit_kenny);
//   }
// };

void
update_ui_combat_turnbased_system(entt::registry& r, const glm::ivec2& input_mouse_pos)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  // Limit: the next bit should only happen if it's not the start of the game
  auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team == AvailableTeams::neutral)
    return;

  const auto& selected_view = r.view<SelectedComponent>();
  int count = (int)selected_view.size();

  // move all guns to the mouse cursor
  // adding a static target component seems weird for only the gun
  for (const auto& [e, selected_c] : selected_view.each()) {
    auto& gun_tgt = r.get_or_emplace<GunStaticTargetComponent>(e);
    gun_tgt.target = { input_mouse_pos.x, input_mouse_pos.y };
  }

  // limit: must be interacting with 1 selected unit
  if (count != 1)
    return;

  // limit: must be player turn
  if (state.team != AvailableTeams::player)
    return;

  // limit: must not be on ui (except loot ui)
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& lootbag_ui = get_first_component<SINGLE_UI_Lootbag>(r);
  if (!ri.viewport_hovered && !lootbag_ui.hovered)
    return;

  const bool paused = get_first_component<SINGLETON_GameStateComponent>(r).state == GameState::PAUSED;
  if (paused)
    return;

  for (const auto& [e, selected_c] : selected_view.each()) {

    auto& actions = r.get_or_emplace<ActionState>(e);

    if (!inside_ship(r, e))
      return; // only move if onboard

    bool request_move = false;
    bool request_shoot = false;
    glm::vec2 move_position{ 0, 0 };
    const auto wp = get_position(r, e);

    {
      // override with keyboard or controller input
      if (auto* inp_c = r.try_get<InputComponent>(e)) {

        request_move |= inp_c->unprocessed_move_down;

        request_shoot |= inp_c->shoot;
        inp_c->shoot = false;

        const auto round_away_from_zero = [](const float value) -> float {
          if (value > 0.0f)
            return std::ceil(value);
          else if (value < 0.0f)
            return std::floor(value);
          else
            return 0.0f;
        };

        if (inp_c->unprocessed_move_down) {
          // aim 1 tile left/right/up/down etc
          move_position.x = wp.x + round_away_from_zero(inp_c->lx) * float(map.tilesize);
          move_position.y = wp.y + round_away_from_zero(inp_c->ly) * float(map.tilesize);
        }

        if (inp_c->unprocessed_move_down)
          inp_c->unprocessed_move_down = false;
      }
    }

    if (move_position == wp)
      return; // moving to same space.. skip

    // move action
    if (actions.actions_available > 0 && request_move) {
      move_action_common(r, e, move_position);

      actions.actions_available--;
      actions.actions_completed++;
    }

    // shoot action
    if (actions.actions_available > 0 && request_shoot) {
      r.emplace_or_replace<WantsToShoot>(e);

      actions.actions_available--;
      actions.actions_completed++;
    }

    if (actions.actions_available == 0)
      create_empty<RequestToCompleteTurn>(r, RequestToCompleteTurn{ AvailableTeams::player });
  }
}

} // namespace game2d