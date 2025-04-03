#include "pch.hpp"

#include "scene_select_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_select/scene_select_components.hpp"

namespace game2d {

void
update_input_for_select_ui(entt::registry& r, SINGLE_SelectSceneData& ui_c)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_state_e, steam_state_c)

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);

  const int joined_players = non_zero_handles(steam_state_c.handles).size();
  for (int i = 0; i < 4; i++) {

    // 4 copies of the ui-state. one per player.
    auto& ui_state_c = ui_c.player_ui_state[i];
    if (!ui_state_c.init) {
      ui_state_c.rows.push_back({ .col_name = "Hull", .action = []() {} });
      ui_state_c.rows.push_back({ .col_name = "Weapon", .action = []() {} });
      ui_state_c.rows.push_back({ .col_name = "Ability", .action = []() {} });

      ui_state_c.init = true;
    }

    // process the input for that ui-state.
    ui_state_c.actions.clear();

    if (i < joined_players)
      process_input_for_ui(r, ui_state_c, steam_state_c.handles[i]);
  }
};

void
draw_select_header(entt::registry& r, SINGLE_RendererInfo& ri_c)
{
  const auto ui_scaling = get_first_component<SINGLE_UIData>(r).scaling;
  const float bar_size_y = 100.0f * ui_scaling;

  ImGui::SetNextWindowPos({ 0, 100 });
  ImGui::SetNextWindowSize({ (float)ri_c.viewport_size_render_at.x, 100.0f }, ImGuiCond_Always);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("SelectHeader", NULL, flags);

  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const auto center = ImVec2{ window_tl.x + 0.5f * window_wh.x, window_tl.y + 0.5f * window_wh.y };

  const auto& io = ImGui::GetIO();

  auto* draw_list = ImGui::GetWindowDrawList();

  // background
  const auto my_bg_col = hex_to_srgb("#245171");
  const auto im_bg_col = IM_COL32(my_bg_col.r, my_bg_col.g, my_bg_col.b, 0.6f * 255);
  const auto window_br = ImVec2{ window_tl.x + window_wh.x, window_tl.y + window_wh.y };
  draw_list->AddRectFilled(window_tl, window_br, im_bg_col, 6);

  // text
  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::MENU_BUTTONS : FontSize::MENU_BUTTONS_SCALED;
  ImGui::PushFont(get_fingerpaint_font(r, font_enum));
  const std::string text_str = "All hands on deck!";
  const auto text_size = ImGui::CalcTextSize(text_str.c_str());
  const auto text_pos = center - ImVec2{ 0.5f * text_size.x, 0.5f * text_size.y };
  draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), text_str.c_str());
  ImGui::PopFont();

  const auto my_bold_col = hex_to_srgb("#497D9D", 255);
  const auto my_dark_col = hex_to_srgb("#09252F");
  const auto im_bold_col = convert_my_to_im(my_bold_col);
  const auto im_dark_col = convert_my_to_im(my_dark_col);

  ImVec2 p_max;

  // health line bold colour
  p_max = ImVec2{ window_tl.x + window_wh.x, window_tl.y + 2 };
  draw_list->AddRectFilled(window_tl, p_max, im_bold_col);

  // health line black line
  auto p_min = ImVec2{ window_tl.x, window_tl.y + bar_size_y - 2 };
  p_max = ImVec2{ window_tl.x + window_wh.x, window_tl.y + bar_size_y };
  draw_list->AddRectFilled(p_min, p_max, im_dark_col);

  ImGui::End();
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

  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::HEADER : FontSize::HEADER_SCALED;
  auto* font = get_fingerpaint_font(r, font_enum);
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