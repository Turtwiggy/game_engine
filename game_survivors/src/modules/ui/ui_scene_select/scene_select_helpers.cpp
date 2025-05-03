#include "pch.hpp"

#include "scene_select_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_player/actor_player_helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui/ui_scene_select/scene_select_components.hpp"

namespace game2d {

void
update_input_for_select_ui(entt::registry& r, SINGLE_SelectSceneData& ui_c)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);

  const int joined_players = (int)non_zero_handles(steam_state_c.handles).size();
  for (int i = 0; i < 4; i++) {

    // 4 copies of the ui-state. one per player.
    auto& ui_state_c = ui_c.player_ui_state[i];

    if (!ui_state_c.init) {

      OptionsCell c;
      c.name = "Hull";
      ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c));

      OptionsCell c1;
      c.name = "Weapon";
      ui_state_c.state.cells.push_back(std::make_shared<OptionsCell>(c1));

      ui_state_c.init = true;
    }

    // process the input for that ui-state.
    if (i < joined_players) {
      const auto input = generate_from_handle(r, steam_state_c.handles[i]);

      ui_state_c.state.actions.clear();
      process_input_for_ui(r, ui_state_c.state, input);
    }
  }

  // keyboard works for player 0
  // process_keyboard_input_for_ui(r, ui_c.player_ui_state[0]);
};

void
update_countdown(SINGLE_SelectSceneData& data_c, bool everyone_confirmed, float dt)
{
  // Start a countdown..
  if (everyone_confirmed)
    data_c.countdown -= dt;
  else
    data_c.countdown = data_c.countdown_max;
};

void
update_countdown_ui(entt::registry& r, const SINGLE_SelectSceneData& data_c)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto x_pos = 0.5f;
  const auto y_pos = 0.75f;
  const auto hmm = ImVec2{ ri.viewport_size_render_at.x * x_pos, ri.viewport_size_render_at.y * y_pos };
  const auto pos = ImVec2{ ri.viewport_pos.x + hmm.x, ri.viewport_pos.y + hmm.y };
  const auto size = ImVec2{ 200, 200 };
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSize(size);

  ImGuiWindowFlags countdown_flags = 0;
  countdown_flags |= ImGuiWindowFlags_NoDecoration;
  countdown_flags |= ImGuiWindowFlags_NoCollapse;
  countdown_flags |= ImGuiWindowFlags_NoTitleBar;
  countdown_flags |= ImGuiWindowFlags_NoBackground;
  countdown_flags |= ImGuiWindowFlags_NoSavedSettings;

  ImGuiIO& io = ImGui::GetIO();

  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::HEADER : FontSize::HEADER_SCALED;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);
  ImGui::Begin("Countdown", NULL, countdown_flags);
  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  const auto str = std::format("{}", ((int)data_c.countdown) + 1);
  const auto text_c_str = str.c_str();
  const auto text_c_str_len = ImGui::CalcTextSize(text_c_str);
  ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (text_c_str_len.x * 0.5));
  ImGui::SetCursorPosY(ui_tl.y + (ui_wh.y * 0.5) - (text_c_str_len.y * 0.5));
  ImGui::Text("%s", text_c_str);

  ImGui::End();
  ImGui::PopFont();
};

void
update_countdown_to_next_scene(entt::registry& r,
                               const SINGLE_SelectSceneData& data_c,
                               const std::vector<ShipHullData>& sorted_hulls)
{
  if (data_c.countdown > 0.0f)
    return;

  SelectSceneToSurviveScene data;
  data.chosen_boats = data_c.player_choice_state;
  create_persistent<SelectSceneToSurviveScene>(r, data);

  move_to_scene_start(r, Scene::survive);
};

} // namespace game2d