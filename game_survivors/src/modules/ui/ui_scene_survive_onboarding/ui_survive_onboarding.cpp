#include "pch.hpp"

#include "ui_survive_onboarding.hpp"
#include "ui_survive_onboarding_components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_input_for_onboarding_ui(entt::registry& r, SINGLE_InfoUI& ui_c, const float dt)
{
  for (int i = 0; i < 4; i++) {
    float& percent = ui_c.player_ready_percents[i];

    auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null) {
      percent = 1.0f; // set non-connected players as ready.
      continue;
    }

    if (percent >= 1.0f)
      continue;

    const auto& inp_c = r.get<InputComponent>(player_e);
    const auto& b_s = inp_c.button_s;
    const bool do_act_held = std::find(b_s.begin(), b_s.end(), ActionStateEnum::HELD) != b_s.end();
    const bool do_act_release = std::find(b_s.begin(), b_s.end(), ActionStateEnum::RELEASE) != b_s.end();

    if (do_act_held)
      percent += dt;

    if (do_act_release || !do_act_held)
      percent = 0.0f;

    percent = glm::clamp(percent, 0.0f, 1.0f);
  }
}

void
update_ui_survive_onboarding_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, steam_ui_e, steam_ui_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c);
  GET_FIRST_OR_RETURN(SINGLE_InfoUI, r, ui_e, ui_c)

  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;

#if defined(_DEBUG)
  ZoneScoped;
#endif

  const int max_num_players = 4;

  if (!ui_c.init) {
    ui_c.player_ready_percents.resize(max_num_players);
    ui_c.init = true;
    return;
  }

  // complete if all percents are 1.0f
  const auto is_complete = [](const float p) { return p >= 1.0f; };
  ui_c.complete = std::all_of(ui_c.player_ready_percents.begin(), ui_c.player_ready_percents.end(), is_complete);
  if (ui_c.complete)
    return;

  update_input_for_onboarding_ui(r, ui_c, dt);

  const float pad_x = 10.0f;
  const float pad_y = 10.0f;

  auto& ri_c = SINGLE_RendererInfo::instance;
  const auto set_window_size = ImVec2{ 0.5f * ri_c.viewport_size_render_at.x, 0.5f * ri_c.viewport_size_render_at.y };
  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  ImGui::SetNextWindowSize(set_window_size);
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  const auto font_enum = ui_scaling == 1.0f ? FontSize::TEXT_SIZE_20 : FontSize::TEXT_SIZE_20_SCALED;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  imgui_begin("SurviveSceneOnboarding", ImGuiWindowFlags_NoInputs);
  const auto window_tl = ImGui::GetWindowPos();
  const auto window_wh = ImGui::GetWindowSize();
  const auto window_br = window_tl + window_wh;
  const auto center_x = window_tl.x + 0.5f * window_wh.x;

  auto* draw_list = ImGui::GetWindowDrawList();

  // draw a background
  const auto my_col = default_player_colours[0];
  const auto im_col = convert_my_to_im(my_col);
  draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, 6);
  draw_list->AddRect(window_tl, window_br, im_col, 6, ImDrawFlags_RoundCornersAll, 2);

  // note: could improve this by using proper icons for the buttons
  ImGui::SeparatorText("Movement");
  ImGui::Text(" Use left analogue stick to move (in water)");
  ImGui::Text(" Use DPAD to move character (on land).");
  ImGui::Text(" Use button(south) to land.");
  ImGui::Text(" Use button(east) to leave island.");
  ImGui::Text(" Use LB to use ability: dash");
  ImGui::Text(" Use RB to use ability: knockback");

  ImGui::NewLine();
  ImGui::SeparatorText("Gameplay");
  ImGui::Text(" Collect XP to level up and choose upgrades.");
  ImGui::Text(" Every 4th weapon level (up to lv 12), choose a new unlock.");
  ImGui::Text(" Clear islands to place lighthouses that provide vision.");

  ImGui::SeparatorText(" Hold Confirm Button to Ready. Goodluck!");

  //
  // Wait for all connected players to confirm.
  // In the bottom center, show 4 squares.
  // When the player has readied up, show a tick.
  //

  const ImVec2 player_wh = { window_wh.x / max_num_players, window_wh.y };
  auto first_tl_x = center_x;
  first_tl_x -= max_num_players * (0.5f * player_wh.x);

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {
    const auto handle = steam_ui_c.handles[player_idx];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(steam_ui_c, handle);

    if (player_idx != 0 && !(connected || joined))
      continue;

    const auto my_player_col = default_player_colours[player_idx];
    const auto im_player_col = convert_my_to_im(my_player_col);
    auto my_player_col_active = my_player_col;
    auto my_player_col_inactive = my_player_col;
    my_player_col_inactive.a = 0.25f * 255;
    const auto im_player_col_active = convert_my_to_im(my_player_col_active);
    const auto im_player_col_inactive = convert_my_to_im(my_player_col_inactive);

    const float bar_rounding = 4.0f;
    const float y_offset = 6.0f;
    const float x_pad = 4;
    const auto center_y = player_wh.y * 0.5f;
    const auto confirm_wh = ImVec2{ player_wh.x - 2.0f * x_pad, 20.0f };
    const auto confirm_tl = ImVec2{ first_tl_x + x_pad, window_br.y - confirm_wh.y - y_offset };
    const auto confirm_br = ImVec2{ first_tl_x - x_pad + confirm_wh.x, confirm_tl.y + confirm_wh.y - y_offset };

    // draw a confirm box
    // draw_list->AddRectFilled(confirm_tl, confirm_br, im_window_bg_col, bar_rounding);
    draw_list->AddRect(confirm_tl, confirm_br, im_player_col, bar_rounding, ImDrawFlags_RoundCornersAll, 2);

    // draw percentage of confirm
    const auto draw_bar = [&](const ImVec2 bar_tl, const ImVec2 bar_br, const ImVec2 bar_wh, const float percent) {
      const ImDrawFlags flags = ImDrawFlags_RoundCornersRight;

      // bar bg
      draw_list->AddRectFilled(bar_tl, bar_br, im_player_col_inactive, bar_rounding, flags);

      // bar fg
      float x = bar_tl.x + percent * bar_wh.x;
      x = glm::min(x, bar_br.x);
      const auto partial_bar_br = ImVec2(x, bar_br.y);
      ImU32 col_l = im_player_col_active;
      ImU32 col_r = im_player_col_inactive;
      draw_list->AddRectFilledMultiColor(bar_tl, partial_bar_br, col_r, col_l, col_l, col_r);
    };

    const float percent = ui_c.player_ready_percents[player_idx];
    draw_bar(confirm_tl, confirm_br, confirm_wh, percent);

    first_tl_x += player_wh.x; // move horizontally
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
  ImGui::PopFont();
}

} // namespace game2d