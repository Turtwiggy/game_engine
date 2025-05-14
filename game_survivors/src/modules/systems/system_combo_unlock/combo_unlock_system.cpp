#include "pch.hpp"

#include "combo_unlock_components.hpp"
#include "combo_unlock_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
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

//
// draw 4 sprites for the unlock code.
// draw 4 sprites for proximity input.
// if input == unlock code, unlock it (i.e. die)
// if player make a mistake, reset the input
//
void
update_combo_unlock_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  static float offset_1st_row = -32;
  static float offset_2nd_row = -64;
  // imgui_draw_float("offset_1st_row", offset_1st_row);
  // imgui_draw_float("offset_2nd_row", offset_2nd_row);

  const auto view = r.view<TreasureEnemyComponent, ComboUnlockComponent>();
  for (const auto& [e, enemy_c, combo_c] : view.each()) {

    // use only inputs from players within range
    const auto pos_pixels = get_position(r, e);
    const auto pos_meters = pixels_to_meters(pos_pixels);
    const auto search_radius_meters = pixels_to_meters(100);
    const auto is_player = [](entt::registry& r, entt::entity parent_e) -> bool {
      return r.try_get<PlayerComponent>(parent_e) != nullptr;
    };
    const auto players_map = get_all_in_area_filtered(r, pos_meters, search_radius_meters, is_player);

    // append all the player inputs.
    for (const auto [player_e, colls] : players_map) {
      const auto& input_c = r.get<const InputComponent>(player_e);
      const auto& b_s = input_c.button_s;
      const auto& b_e = input_c.button_e;
      const auto& b_n = input_c.button_n;
      const auto& b_w = input_c.button_w;
      const bool dir_d = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();
      const bool dir_u = std::find(b_n.begin(), b_n.end(), ActionStateEnum::DOWN) != b_n.end();
      const bool dir_l = std::find(b_w.begin(), b_w.end(), ActionStateEnum::DOWN) != b_w.end();
      const bool dir_r = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();
      if (dir_d)
        combo_c.current.push_back(COMBO_DIR::D);
      if (dir_u)
        combo_c.current.push_back(COMBO_DIR::U);
      if (dir_l)
        combo_c.current.push_back(COMBO_DIR::L);
      if (dir_r)
        combo_c.current.push_back(COMBO_DIR::R);
    }

    // dont display if not near
    if (players_map.size() == 0)
      return;

    // display players inputs.
    for (int i = 0; i < combo_c.current.size(); i++) {
      auto val = combo_c.current[i];

      Sprite debug_s;
      debug_s.pos = pos_pixels;
      debug_s.pos.x += 32 * i;
      debug_s.pos.y += offset_2nd_row;
      debug_s.size = { 32, 32 };
      debug_s.z_rotation = 0.0f;
      debug_s.sprite = get_sprite_for_combodir(val);
      debug_s.col = engine::SRGBColour{ 0, 255, 0, 255 };
      draw_sprite(r, debug_s);
    }

    // display the code.
    for (int i = 0; i < combo_c.unlock.size(); i++) {
      auto val = combo_c.unlock[i];

      Sprite debug_s;
      debug_s.pos = pos_pixels;
      debug_s.pos.x += 32 * i;
      debug_s.pos.y += offset_1st_row;
      debug_s.size = { 32, 32 };
      debug_s.z_rotation = 0.0f;
      debug_s.sprite = get_sprite_for_combodir(val);
      debug_s.col = engine::SRGBColour{ 255, 255, 255, 255 };
      draw_sprite(r, debug_s);
    }

    // validate inputs; reset if an invalid input is entered.
    const auto& u = combo_c.unlock;
    const auto& i = combo_c.current;
    bool is_valid = true;
    is_valid &= i.size() <= u.size();
    is_valid &= std::equal(i.begin(), i.end(), u.begin()); // order must match
    if (!is_valid)
      combo_c.current.clear();

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