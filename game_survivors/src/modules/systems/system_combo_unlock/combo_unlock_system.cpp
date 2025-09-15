#include "pch.hpp"

#include "combo_unlock_components.hpp"
#include "combo_unlock_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_enemy_treasure/enemy_treasure_components.hpp"
#include "modules/actors/actor_player/components.hpp"

namespace game2d {

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
  static float offset_2nd_row = -32;
  // imgui_draw_float("offset_1st_row", offset_1st_row);
  // imgui_draw_float("offset_2nd_row", offset_2nd_row);

  const auto view = r.view<TreasureEnemyComponent, ComboUnlockComponent>();
  for (const auto& [e, enemy_c, combo_c] : view.each()) {

    // use only inputs from players within range
    const auto pos_pixels = get_position(r, e);
    const auto pos_meters = pixels_to_meters(pos_pixels);
    const auto search_radius_meters = pixels_to_meters(100);
    const auto is_player = [](entt::registry& r, entt::entity parent_e) -> bool {
      return r.all_of<PlayerBoatComponent>(parent_e);
    };
    const auto players_map = get_all_in_area_filtered(r, pos_meters, search_radius_meters, is_player);

    // append all the player inputs.
    for (const auto [player_e, colls] : players_map) {
      const auto& input_c = r.get<const InputComponent>(player_e);
      const auto& b_s = input_c.dpad_d;
      const auto& b_e = input_c.dpad_r;
      const auto& b_n = input_c.dpad_u;
      const auto& b_w = input_c.dpad_l;
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
    combo_c.display = false;
    if (players_map.empty())
      return;
    combo_c.display = true;
  }
}

} // namespace game2d