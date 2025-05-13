#include "pch.hpp"

#include "combo_unlock_components.hpp"
#include "combo_unlock_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_death/components.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

std::string
get_sprite_for_combodir(COMBO_DIR dir)
{
  if (dir == COMBO_DIR::U)
    return "ARROW_UP";

  else if (dir == COMBO_DIR::D)
    return "ARROW_DOWN";

  else if (dir == COMBO_DIR::L)
    return "ARROW_LEFT";

  else
    return "ARROW_RIGHT";
};

void
update_combo_unlock_system(entt::registry& r)
{
  static float offset_1st_row = -32;
  static float offset_2nd_row = -64;
  imgui_draw_float("offset_1st_row", offset_1st_row);
  imgui_draw_float("offset_2nd_row", offset_2nd_row);

  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const auto& b_e = g_input_c.button_e;
  const auto& b_n = g_input_c.button_n;
  const auto& b_w = g_input_c.button_w;
  const bool dir_d = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();
  const bool dir_u = std::find(b_n.begin(), b_n.end(), ActionStateEnum::DOWN) != b_n.end();
  const bool dir_l = std::find(b_w.begin(), b_w.end(), ActionStateEnum::DOWN) != b_w.end();
  const bool dir_r = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();

  const auto view = r.view<TreasureEnemyComponent, ComboUnlockComponent>();
  for (const auto& [e, enemy_c, combo_c] : view.each()) {

    //
    // draw 4 sprite for the unlock code.
    // draw 4 sprite for your current input.
    // if you match, unlock it (i.e. die)
    // if you make a mistake, reset the input
    //
    // get all players within range.
    // get all u/d/l/r inputs from those players.
    // append input to the combo_c.current
    // if the combo_c.current matches combo_c.code, "unlock" the chest
    //

    // hack: just use global inputs.
    // todo: use only inputs from players within range

    if (dir_d)
      combo_c.current.push_back(COMBO_DIR::D);
    if (dir_u)
      combo_c.current.push_back(COMBO_DIR::U);
    if (dir_l)
      combo_c.current.push_back(COMBO_DIR::L);
    if (dir_r)
      combo_c.current.push_back(COMBO_DIR::R);

    const auto pos = get_position(r, e);

    // display players inputs.
    for (int i = 0; i < combo_c.current.size(); i++) {
      auto val = combo_c.current[i];

      Sprite debug_s;
      debug_s.pos = pos;
      debug_s.pos.x += 32 * i;
      debug_s.pos.y += offset_2nd_row;
      debug_s.size = { 32, 32 };
      debug_s.sprite = get_sprite_for_combodir(val);
      debug_s.col = engine::SRGBColour{ 0, 255, 0, 255 };
      draw_sprite(r, debug_s);
    }

    // display the code.
    for (int i = 0; i < combo_c.unlock.size(); i++) {
      auto val = combo_c.unlock[i];

      Sprite debug_s;
      debug_s.pos = pos;
      debug_s.pos.x += 32 * i;
      debug_s.pos.y += offset_1st_row;
      debug_s.size = { 32, 32 };
      debug_s.sprite = get_sprite_for_combodir(val);
      debug_s.col = engine::SRGBColour{ 255, 255, 255, 255 };
      draw_sprite(r, debug_s);
    }

    // unlock the chest!
    if (combo_c.current == combo_c.unlock) {
      auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);

      // Send death event.
      DeathEvent d_evt;
      d_evt.killed_by = entt::null;
      d_evt.dead = e; // parent not fixture
      auto& evts = get_first_component<SINGLE_Events>(r);
      evts.dispatcher->trigger(d_evt);
      evts.dispatcher->update();

      dead_c.dead.push_back(e);
    }
  }
}

} // namespace game2d