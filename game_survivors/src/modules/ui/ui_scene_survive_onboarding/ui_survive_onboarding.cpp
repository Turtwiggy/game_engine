#include "pch.hpp"

#include "ui_survive_onboarding.hpp"
#include "ui_survive_onboarding_components.hpp"
#include "ui_survive_onboarding_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {
using namespace std::literals;

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
  // const auto set_window_size = ImVec2{ 0.5f * 1280, 0.5f * 720 };
  const auto set_window_pos = ImVec2{ ri_c.viewport_size_render_at.x * 0.5f, ri_c.viewport_size_render_at.y * 0.5f };
  ImGui::SetNextWindowSize(set_window_size);
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  const auto font_enum_16 = ui_scaling == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto font_enum_20 = ui_scaling == 1.0f ? FontSize::TEXT_SIZE_20 : FontSize::TEXT_SIZE_20_SCALED;
  auto* font_16 = get_inter_font(r, font_enum_16);
  auto* font_20 = get_inter_font(r, font_enum_20);

  imgui_begin("SurviveSceneOnboarding", ImGuiWindowFlags_NoInputs);
  const auto window_tl = ImGui::GetWindowPos();
  const auto window_wh = ImGui::GetWindowSize();
  const auto window_br = window_tl + window_wh;

  auto* draw_list = ImGui::GetWindowDrawList();

  // draw a background
  const auto my_col = default_player_colours[0];
  const auto im_col = convert_my_to_im(my_col);
  draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, 6);
  draw_list->AddRect(window_tl, window_br, im_col, 6, ImDrawFlags_RoundCornersAll, 1.5f);

  ImGui::PushFont(font_20);
  ImGui::SeparatorText("Movement");
  ImGui::PopFont();

  // note: could improve this by using proper icons for the buttons

  // clang-format off
  ImGui::PushFont(font_16);
  ImGui::SetCursorPosX(0.1f * window_wh.x); ImGui::Text(" Use left analogue stick to move (in water)");
  ImGui::SetCursorPosX(0.1f * window_wh.x); ImGui::Text(" Use DPAD to move character (on land).");
  ImGui::SetCursorPosX(0.1f * window_wh.x); ImGui::Text(" Use button(south) to land.");
  ImGui::SetCursorPosX(0.1f * window_wh.x); ImGui::Text(" Use button(east) to leave island.");
  ImGui::SetCursorPosX(0.1f * window_wh.x); ImGui::Text(" Use LB to use ability: Dash");
  ImGui::SetCursorPosX(0.1f * window_wh.x); ImGui::Text(" Use RB to use ability: Knockback");
  ImGui::PopFont();
  // clang-format on

  ImGui::NewLine();
  ImGui::PushFont(font_20);
  ImGui::SeparatorText("Gameplay");
  ImGui::PopFont();

  // clang-format off
  ImGui::PushFont(font_16);
  ImGui::SetCursorPosX(0.1f * window_wh.x);ImGui::Text(" Collect XP to level up and choose upgrades.");
  ImGui::SetCursorPosX(0.1f * window_wh.x);ImGui::Text(" Every 4th weapon level (up to lv 12), choose a new unlock.");
  ImGui::SetCursorPosX(0.1f * window_wh.x);ImGui::Text(" Capture islands to place lighthouses that provide vision.");
  ImGui::PopFont();
  // clang-format on

  //
  // Wait for all connected players to confirm.
  // In the bottom center, show 4 squares.
  // When the player has readied up, show a tick.
  //

  const auto center_x = window_tl.x + 0.5f * window_wh.x;
  const ImVec2 player_wh = { window_wh.x / max_num_players, window_wh.y };
  auto first_tl_x = center_x;
  first_tl_x -= max_num_players * (0.5f * player_wh.x);

  for (int player_idx = 0; player_idx < max_num_players; player_idx++) {
    const auto handle = steam_ui_c.handles[player_idx];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(steam_ui_c, handle);

    const auto my_player_col = default_player_colours[player_idx];
    const auto im_player_col = convert_my_to_im(my_player_col);
    auto my_player_col_active = my_player_col;
    auto my_player_col_inactive = my_player_col;
    my_player_col_inactive.a = 0.25f * 255;
    const auto im_player_col_active = convert_my_to_im(my_player_col_active);
    const auto im_player_col_inactive = convert_my_to_im(my_player_col_inactive);

    const float y_offset = 6.0f;
    const float x_pad = 10.0f;
    const float bar_height = font_20->FontSize + 4.0f;
    const float bar_rounding = 0.0f;

    const auto confirm_wh = ImVec2{ player_wh.x, bar_height };
    const auto confirm_tl = ImVec2{ first_tl_x + x_pad, window_br.y - confirm_wh.y - y_offset };
    const auto confirm_br = ImVec2{ first_tl_x - x_pad + confirm_wh.x, confirm_tl.y + confirm_wh.y - y_offset };

    // draw a confirm box
    // draw_list->AddRectFilled(confirm_tl, confirm_br, im_window_bg_col, bar_rounding);
    draw_list->AddRect(confirm_tl, confirm_br, im_player_col, bar_rounding, ImDrawFlags_RoundCornersAll, 1);

    // draw percentage of confirm
    const auto draw_bar = [&](const ImVec2 bar_tl, const ImVec2 bar_br, const float percent) {
      // const ImDrawFlags flags = ImDrawFlags_RoundCornersRight;
      const ImDrawFlags flags = 0;

      // bar bg
      draw_list->AddRectFilled(bar_tl, bar_br, im_player_col_inactive, bar_rounding, flags);

      // bar fg
      const ImVec2 bar_wh = bar_br - bar_tl;
      float x = bar_tl.x + percent * bar_wh.x;
      x = glm::min(x, bar_br.x);
      const auto partial_bar_br = ImVec2(x, bar_br.y);
      ImU32 col_l = im_player_col_active;
      ImU32 col_r = im_player_col_inactive;
      draw_list->AddRectFilledMultiColor(bar_tl, partial_bar_br, col_r, col_l, col_l, col_r);
    };

    const float percent = ui_c.player_ready_percents[player_idx];
    const ImVec2 bar_wh = confirm_br - confirm_tl;
    draw_bar(confirm_tl, confirm_br, percent);

    auto ready_text = std::format("Hold {}", get_confirm_button_str(r, handle));
    if (player_idx != 0 && !joined)
      ready_text = "Not connected";

    static auto padding_x = 4.0f;
    static auto padding_y = 1.8f;
    // ImGui::Begin("temp");
    // imgui_draw_float("padding_x", padding_x);
    // imgui_draw_float("padding_y", padding_y);
    // ImGui::End();

    const float icon_pad_x = 2.0f;

    // circle icon
    if (percent < 1.0f) {
      const ImVec2 icon_size{ bar_wh.y, bar_wh.y };
      auto icon_pos = confirm_tl + ImVec2{ 0.5f * icon_size.x, 0.5f * icon_size.y };
      icon_pos.x += 2.0f;
      draw_list->AddCircle(icon_pos, 0.33f * icon_size.x, im_text_col, 16, 3.0f);
    }
    // draw a tick icon
    else {
      const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "kenneynl_gameicons")->id;
      const auto im_id = (ImTextureID)(void*)(intptr_t)tex_id;
      const ImVec2 icon_size{ bar_wh.y, bar_wh.y };
      const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, "ICON_TICK"s);
      auto icon_pos = confirm_tl;
      icon_pos.x += icon_pad_x;
      draw_list->AddImage(im_id, icon_pos, icon_pos + icon_size, icon_tl, icon_br);
    }

    const auto ready_text_size = font_16->CalcTextSizeA(font_16->FontSize, FLT_MAX, -1, ready_text.c_str());
    auto text_pos = ImVec2{ confirm_tl.x + padding_x, confirm_tl.y + padding_y };
    text_pos.x += 0.5f * (bar_wh.x - ready_text_size.x);
    // draw_list->AddText(font_16, font_16->FontSize, text_pos, im_player_col_active, ready_text.c_str());
    draw_list->AddText(font_16, font_16->FontSize, text_pos, im_text_col, ready_text.c_str());

    // move horizontally
    first_tl_x += player_wh.x;
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d