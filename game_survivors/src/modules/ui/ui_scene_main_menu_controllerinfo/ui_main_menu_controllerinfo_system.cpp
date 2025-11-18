#include "pch.hpp"

#include "ui_main_menu_controllerinfo_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/maths.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "resources/data.hpp"

namespace game2d {
using namespace std::literals;

enum class ControllerState
{
  CONNECTED = 0,
  DISCONNECTED,
  NOT_CONNECTED,

  count,
};

// ControllerState
// get_controller_state_from_handle(InputHandle_t handle)
// {
//   return ControllerState::NOT_CONNECTED;
// };

ImColor
get_button_col(const SINGLE_SteamControllers& steam_c, const InputHandle_t handle, const DigitalAction da, const int alpha)
{
  // if active, white
  if (controller_button_held(steam_c, handle, da))
    return ImColor(255, 255, 255, alpha);
  // if not active, black
  return ImColor(0, 0, 0, alpha);
};

void
draw_dpad(const SINGLE_SteamControllers& steam_c,
          const InputHandle_t handle,
          auto* draw_list,
          const ImVec2 tl,
          const ImVec2 wh,
          const float ui_scale,
          const int alpha)
{
  // dpad
  const float rect_rad = 6.0f * ui_scale;
  const float rect_spacing = 1.0f * ui_scale;

  // draw some squares for the dpad.
  const auto dpad_center = ImVec2{ tl.x + (0.75f / 6.0f) * wh.x, tl.y + (0.5f * wh.y) };
  const auto dpad_center_tl = ImVec2(dpad_center.x - rect_rad, dpad_center.y - rect_rad);
  const auto dpad_center_br = ImVec2(dpad_center.x + rect_rad, dpad_center.y + rect_rad);
  draw_list->AddRectFilled(dpad_center_tl, dpad_center_br, IM_COL32(0, 0, 0, alpha));

  // dpad-left
  {
    const auto dpad_l_center_tl = ImVec2(dpad_center.x - 3.0f * rect_rad - rect_spacing, dpad_center.y - rect_rad);
    const auto dpad_l_center_br = ImVec2(dpad_center.x - 1.0f * rect_rad - rect_spacing, dpad_center.y + rect_rad);
    draw_list->AddRectFilled(dpad_l_center_tl, dpad_l_center_br, get_button_col(steam_c, handle, DA::Game_Left, alpha));
  }
  // dpad-right
  {
    const auto dpad_r_center_tl = ImVec2(dpad_center.x + 1.0f * rect_rad + rect_spacing, dpad_center.y - rect_rad);
    const auto dpad_r_center_br = ImVec2(dpad_center.x + 3.0f * rect_rad + rect_spacing, dpad_center.y + rect_rad);
    draw_list->AddRectFilled(dpad_r_center_tl, dpad_r_center_br, get_button_col(steam_c, handle, DA::Game_Right, alpha));
  }
  // dpad-up
  {
    const auto dpad_u_center_tl = ImVec2(dpad_center.x - rect_rad, dpad_center.y - 3.0f * rect_rad - rect_spacing);
    const auto dpad_u_center_br = ImVec2(dpad_center.x + rect_rad, dpad_center.y - 1.0f * rect_rad - rect_spacing);
    draw_list->AddRectFilled(dpad_u_center_tl, dpad_u_center_br, get_button_col(steam_c, handle, DA::Game_Up, alpha));
  }
  // dpad-down
  {
    const auto dpad_d_center_tl = ImVec2(dpad_center.x - rect_rad, dpad_center.y + 1.0f * rect_rad + rect_spacing);
    const auto dpad_d_center_br = ImVec2(dpad_center.x + rect_rad, dpad_center.y + 3.0f * rect_rad + rect_spacing);
    draw_list->AddRectFilled(dpad_d_center_tl, dpad_d_center_br, get_button_col(steam_c, handle, DA::Game_Down, alpha));
  }
};

void
draw_abxy_buttons(entt::registry& r,
                  const SINGLE_SteamControllers& steam_c,
                  const InputHandle_t handle,
                  auto* draw_list,
                  const ImVec2 tl,
                  const ImVec2 wh,
                  const float ui_scale,
                  const int alpha)
{
  // abxy
  const float abxy_radius = 8.0f * ui_scale;

  // ABXY
  const auto u_pos = ImVec2(tl.x + (10.1f / 12.0f) * wh.x, tl.y + (4.8f / 12.0f) * wh.y);
  const auto d_pos = ImVec2(tl.x + (10.4f / 12.0f) * wh.x, tl.y + (7.2f / 12.0f) * wh.y);
  const auto l_pos = ImVec2(tl.x + (9.5f / 12.0f) * wh.x, tl.y + (6.5f / 12.0f) * wh.y);
  const auto r_pos = ImVec2(tl.x + (11.0f / 12.0f) * wh.x, tl.y + (5.5f / 12.0f) * wh.y);
  draw_list->AddCircleFilled(u_pos, abxy_radius, get_button_col(steam_c, handle, DA::Game_North, alpha));
  draw_list->AddCircleFilled(d_pos, abxy_radius, get_button_col(steam_c, handle, DA::Game_South, alpha));
  draw_list->AddCircleFilled(l_pos, abxy_radius, get_button_col(steam_c, handle, DA::Game_West, alpha));
  draw_list->AddCircleFilled(r_pos, abxy_radius, get_button_col(steam_c, handle, DA::Game_East, alpha));

  add_text_centered(r, draw_list, get_str_for_da(steam_c, handle, DA::Game_North), u_pos, alpha);
  add_text_centered(r, draw_list, get_str_for_da(steam_c, handle, DA::Game_West), l_pos, alpha);
  add_text_centered(r, draw_list, get_str_for_da(steam_c, handle, DA::Game_East), r_pos, alpha);
  add_text_centered(r, draw_list, get_str_for_da(steam_c, handle, DA::Game_South), d_pos, alpha);
};

void
draw_bumpers(entt::registry& r,
             const SINGLE_SteamControllers& steam_c,
             const InputHandle_t handle,
             auto* draw_list,
             const ImVec2 tl,
             const ImVec2 wh,
             const float ui_scale,
             const int alpha)
{
  const float bumper_rounding = 8;

  // lb
  {
    const auto lb_offset = ImVec2{ 2, 2 };
    const auto lb_tl = ImVec2{ tl.x + (0 / 6.0f) * wh.x + lb_offset.x, tl.y + (0 / 6.0f) * wh.y + lb_offset.y };
    const auto lb_br = ImVec2{ tl.x + (2.5f / 6.0f) * wh.x + lb_offset.x, tl.y + (1 / 6.0f) * wh.y + lb_offset.y };
    const auto lb_center = ImVec2{ lb_tl.x + 0.5f * (lb_br.x - lb_tl.x), lb_tl.y + 0.5f * (lb_br.y - lb_tl.y) };
    draw_list->AddRectFilled(lb_tl, lb_br, get_button_col(steam_c, handle, DA::Game_LB, alpha), bumper_rounding);

    const auto l_shoulder_txt = get_str_for_da(steam_c, handle, DA::Game_LB);
    add_text_centered(r, draw_list, l_shoulder_txt, lb_center, alpha);
  }

  // rb
  {
    const auto rb_offset = ImVec2{ -2, 2 };
    const auto rb_tl = ImVec2{ tl.x + ((6 - 2.5f) / 6.0f) * wh.x + rb_offset.x, tl.y + (0 / 6.0f) * wh.y + rb_offset.y };
    const auto rb_br = ImVec2{ tl.x + (6 / 6.0f) * wh.x + rb_offset.x, tl.y + (1 / 6.0f) * wh.y + rb_offset.y };
    const auto rb_center = ImVec2{ rb_tl.x + 0.5f * (rb_br.x - rb_tl.x), rb_tl.y + 0.5f * (rb_br.y - rb_tl.y) };
    draw_list->AddRectFilled(rb_tl, rb_br, get_button_col(steam_c, handle, DA::Game_RB, alpha), bumper_rounding);

    const auto r_shoulder_txt = get_str_for_da(steam_c, handle, DA::Game_RB);
    add_text_centered(r, draw_list, r_shoulder_txt, rb_center, alpha);
  }
}

void
draw_eyebrows(const SINGLE_SteamControllers& steam_c,
              const InputHandle_t handle,
              auto* draw_list,
              const ImVec2 tl,
              const ImVec2 wh,
              const float ui_scale,
              const int alpha)
{
  const float eyebrow_thickness = 8.0f * ui_scale;

  // draw some angry eyebrows
  // representing the start/select button
  const auto l_eyebrow_p1 = ImVec2{ tl.x + (2 / 6.0f * wh.x), tl.y + (1 / 3.0f * wh.y) };
  const auto l_eyebrow_p2 = ImVec2{ tl.x + (0.48f * wh.x), tl.y + (0.5f * wh.y) };
  const auto r_eyebrow_p1 = ImVec2{ tl.x + (4 / 6.0f * wh.x), tl.y + (1 / 3.0f * wh.y) };
  const auto r_eyebrow_p2 = ImVec2{ tl.x + (0.52f * wh.x), tl.y + (0.5f * wh.y) };
  draw_list->AddLine(l_eyebrow_p1, l_eyebrow_p2, get_button_col(steam_c, handle, DA::Game_Back, alpha), eyebrow_thickness);
  draw_list->AddLine(r_eyebrow_p1, r_eyebrow_p2, get_button_col(steam_c, handle, DA::Game_Start, alpha), eyebrow_thickness);
}

void
draw_eyes(entt::registry& r,
          const SINGLE_SteamControllers& steam_c,
          const InputHandle_t handle,
          auto* draw_list,
          const ImVec2 tl,
          const ImVec2 wh,
          const float ui_scale,
          const int player_idx,
          const int alpha)
{
  // draw a circle representing an eye/analogue
  const auto my_col = default_player_colours[player_idx];
  const auto eye_col = ImColor(0.0f, 0.0f, 0.0f, alpha / 255.0f);
  const auto eyesocket_col = IM_COL32(0.8f * my_col.r, 0.8f * my_col.g, 0.8f * my_col.b, alpha);
  const auto eye_spec_col = ImColor(1.0f, 1.0f, 1.0f, alpha / 255.0f);
  const float eye_radius = 17.0 * ui_scale;
  const float eye_spec_radius = 2.0f * ui_scale;
  const ImVec2 eye_spec_offset = { 6 * ui_scale, -6 * ui_scale };

  // get the inputs...
  const auto l_analog = controller_axis(r, handle, AA::LAnalogControls);
  const auto r_analog = controller_axis(r, handle, AA::RAnalogControls);

  const auto strength = ImVec2(10.0f, 10.0f);
  const auto l_center = ImVec2{ tl.x + (0.334f * wh.x), tl.y + (0.667f * wh.y) };
  const auto r_center = ImVec2{ tl.x + (0.667f * wh.x), tl.y + (0.667f * wh.y) };
  const auto l_center_input = ImVec2{ l_center.x + strength.x * l_analog.x, l_center.y + strength.y * -l_analog.y };
  const auto r_center_input = ImVec2{ r_center.x + strength.x * r_analog.x, r_center.y + strength.y * -r_analog.y };

  // stationary grey circles
  draw_list->AddCircleFilled(l_center, eye_radius, eyesocket_col);
  draw_list->AddCircleFilled(r_center, eye_radius, eyesocket_col);

  // wiggly eyes
  draw_list->AddCircleFilled(l_center_input, eye_radius, eye_col);
  draw_list->AddCircleFilled(r_center_input, eye_radius, eye_col);

  // wiggly eyes specular highlights
  const ImVec2 l_center_with_specular_offset = { l_center_input.x + eye_spec_offset.x,
                                                 l_center_input.y + eye_spec_offset.y };
  const ImVec2 r_center_with_specular_offset = { r_center_input.x + eye_spec_offset.x,
                                                 r_center_input.y + eye_spec_offset.y };
  draw_list->AddCircleFilled(l_center_with_specular_offset, eye_spec_radius, eye_spec_col);
  draw_list->AddCircleFilled(r_center_with_specular_offset, eye_spec_radius, eye_spec_col);
};

void
draw_mouth(const SINGLE_SteamControllers& steam_c,
           const InputHandle_t handle,
           auto* draw_list,
           const ImVec2 tl,
           const ImVec2 wh,
           const float ui_scale,
           const int alpha)
{
  // mouth
  const float mouth_radius_w = 10.0f * ui_scale;
  const float mouth_radius_h = 10.0f * ui_scale;

  const auto rb_tl = ImVec2{ tl.x + 0.5f * wh.x - mouth_radius_w, tl.y + (5 / 6.0f) * wh.y - (0.2f * mouth_radius_w) };
  const auto rb_br = ImVec2{ tl.x + 0.5f * wh.x + mouth_radius_w, tl.y + (5 / 6.0f) * wh.y + (0.2f * mouth_radius_h) };
  draw_list->AddRectFilled(rb_tl, rb_br, IM_COL32(0, 0, 0, alpha), 8);
};

void
draw_player_ui_box(entt::registry& r,
                   const ImVec2 tl,
                   const ImVec2 wh,
                   const int player_idx,
                   const SINGLE_SteamControllers& steam_c,
                   const InputHandle_t handle,
                   const ControllerState state,
                   const float dt)
{
  const auto ui_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  auto& data_c = get_first_component<SINGLE_MainMenuAnimatedData>(r);
  auto& anim_data = data_c.data[player_idx];

  // data
  const auto my_col = default_player_colours[player_idx];
  float alpha = 0.9f;
  if (state == ControllerState::NOT_CONNECTED)
    alpha = 0.5f;
  const auto im_bg_col = IM_COL32(my_col.r, my_col.g, my_col.b, 255 * alpha);
  const int alpha_int = (int)(255 * alpha);

  // initialize some anim data
  static engine::RandomState rnd;
  if (!anim_data.init) {
    anim_data.mask_string_l_y = engine::rand_det_s(rnd.rng, anim_data.mask_min_rnd, anim_data.mask_max_rnd);
    anim_data.mask_string_r_y = engine::rand_det_s(rnd.rng, anim_data.mask_min_rnd, anim_data.mask_max_rnd);
    anim_data.sleeping_mask_y_timer = player_idx; // offset the zzzs
    anim_data.init = true;
  }

  // black separator bar
  const float bar_size = 3.0f * ui_scale;

  // set font
  auto* draw_list = ImGui::GetWindowDrawList();

  // background
  const float inc = ((player_idx + 1) / 4.0f);
  auto p_max = ImVec2{ tl.x + wh.x, tl.y + wh.y };
  draw_list->AddRectFilled(tl, p_max, im_bg_col, 2);

  const auto add_bottom_left_text = [&](std::string text_str) -> void {
    // const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
    const auto font_size = (float)FontSizes::SIZE_12;
    auto* font = get_inter_font(r);
    ImGui::PushFont(font, font_size);

    const auto text_str_len = ImGui::CalcTextSize(text_str.c_str());
    const auto text_size = ImGui::CalcTextSize(text_str.c_str());
    const auto text_offset{ ImVec2{ 4, -4 } };
    const auto text_pos = ImVec2{
      tl.x + text_offset.x,                         // left x
      tl.y + wh.y + (-text_size.y) + text_offset.y, // bottom y
    };

    draw_list->AddText(text_pos, IM_COL32(0, 0, 0, 255), text_str.c_str());
    ImGui::PopFont();
  };

  const auto show_connected_ui = [&]() {
    //
    add_bottom_left_text("Connected");

    draw_eyes(r, steam_c, handle, draw_list, tl, wh, ui_scale, player_idx, alpha_int);
    draw_eyebrows(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_dpad(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_abxy_buttons(r, steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_bumpers(r, steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_mouth(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
  };

  const auto show_disconnected_ui = [&]() {
    //
    add_bottom_left_text("Disconnected");

    // draw eyes as crosses
    const auto l_center = ImVec2{ tl.x + (0.4f * wh.x), tl.y + (0.667f * wh.y) };
    const auto r_center = ImVec2{ tl.x + (0.6f * wh.x), tl.y + (0.667f * wh.y) };
    // const auto text_col = IM_COL32(255, 255, 255, alpha_int);
    const auto text_col = IM_COL32(0, 0, 0, alpha_int);

    const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
    const auto font_size = (float)FontSizes::SIZE_20 * font_scale;
    auto* font = get_inter_font(r);
    ImGui::PushFont(font, font_size);

    {
      const std::string text = "G";
      const auto l_pos = l_center;
      const auto text_wh = ImGui::CalcTextSize(text.c_str());
      const auto text_pos = l_pos - ImVec2{ 0.5f * text_wh.x, 0.5f * text_wh.y };
      draw_list->AddText(text_pos, text_col, text.c_str());
    }
    {
      const std::string text = "G";
      const auto r_pos = r_center;
      const auto text_wh = ImGui::CalcTextSize(text.c_str());
      const auto text_pos = r_pos - ImVec2{ 0.5f * text_wh.x, 0.5f * text_wh.y };
      draw_list->AddText(text_pos, text_col, text.c_str());
    }
    ImGui::PopFont();

    draw_eyebrows(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_mouth(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_dpad(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_abxy_buttons(r, steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_bumpers(r, steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
  };

  const auto show_not_connected_ui = [&]() {
    //
    add_bottom_left_text("Not Connected");

    draw_eyebrows(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_mouth(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_dpad(steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);
    draw_abxy_buttons(r, steam_c, handle, draw_list, tl, wh, ui_scale, alpha_int);

    // draw sleep mask
    const float mask_rounding = 16.0f;
    const auto mask_tl = ImVec2{ tl.x + (3.0f / 12.0f * wh.x), tl.y + (2.5f / 6.0f * wh.y) };
    const auto mask_br = ImVec2{ tl.x + (9.0f / 12.0f * wh.x), tl.y + (4.5f / 6.0f * wh.y) };
    draw_list->AddRectFilled(mask_tl, mask_br, IM_COL32(0, 0, 0, 1.0f * 255), mask_rounding);

    // draw some white lines to represent sleeping mask strings
    const float line_thickness = 2.0f;
    const auto line_col = IM_COL32(255, 255, 255, 255);
    {
      const auto l_line_p0 = ImVec2(tl.x, tl.y + (anim_data.mask_string_l_y * wh.y));
      const auto l_line_p1 = ImVec2(mask_tl.x + 10, tl.y + (0.60f * wh.y));
      const auto r_line_p0 = ImVec2(mask_br.x - 10, tl.y + (0.50f * wh.y));
      const auto r_line_p1 = ImVec2(tl.x + wh.x, tl.y + (anim_data.mask_string_r_y * wh.y));
      draw_list->AddLine(l_line_p0, l_line_p1, line_col, line_thickness);
      draw_list->AddLine(r_line_p0, r_line_p1, line_col, line_thickness);
    }

    // draw some white lines to represent eyes
    {
      const auto l_line_p0 = ImVec2(tl.x + (4.5f / 12.0f) * wh.x, tl.y + (3.5f / 6.0f) * wh.y);
      const auto l_line_p1 = ImVec2(tl.x + (5.5f / 12.0f) * wh.x, tl.y + (3.5f / 6.0f) * wh.y);
      const auto r_line_p0 = ImVec2(tl.x + (6.5f / 12.0f) * wh.x, tl.y + (3.5f / 6.0f) * wh.y);
      const auto r_line_p1 = ImVec2(tl.x + (7.5f / 12.0f) * wh.x, tl.y + (3.5f / 6.0f) * wh.y);
      draw_list->AddLine(l_line_p0, l_line_p1, line_col, line_thickness);
      draw_list->AddLine(r_line_p0, r_line_p1, line_col, line_thickness);
    }

    // Add some Zzz to represent sleeping.
    {
      const auto text = "Zzz"s;

      // make it bob
      const float amplitude = 10.0f;
      const float speed = 2.0f;
      anim_data.sleeping_mask_y_timer += dt * speed;
      const float bob_val = glm::sin(anim_data.sleeping_mask_y_timer) * amplitude;

      const auto pos = ImVec2(mask_br.x, mask_tl.y + bob_val);

      const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
      const auto font_size = (float)FontSizes::SIZE_16 * font_scale;
      auto* font = get_inter_font(r);
      ImGui::PushFont(font, font_size);

      const auto text_wh = ImGui::CalcTextSize(text.c_str());
      const auto text_pos = pos - ImVec2{ 0.5f * text_wh.x, 0.5f * text_wh.y };
      draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), text.c_str());

      ImGui::PopFont();
    }
  };

  if (state == ControllerState::CONNECTED)
    show_connected_ui();
  if (state == ControllerState::DISCONNECTED)
    show_disconnected_ui();
  if (state == ControllerState::NOT_CONNECTED)
    show_not_connected_ui();

  // Draw a black bar at the bottom.
  const auto bar_tl = ImVec2{ tl.x, tl.y + wh.y - bar_size }; // starting at the bl of the ui
  const auto bar_br = ImVec2{ tl.x + wh.x, tl.y + wh.y };
  draw_list->AddRectFilled(bar_tl, bar_br, IM_COL32(0, 0, 0, 255));
}

void
update_ui_scene_main_menu_controllerinfo_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  static float timer = 0.0f;
  static float timer_inc = 0.0f;

  const auto& scene_c = SINGLE_CurrentScene::instance;
  if (scene_c.s != Scene::menu) {
    timer = 0.0f;
    timer_inc = 0.0f;
    return;
  }

  // hack: dont show this menu if options or upgrade is open
  {
    const auto menu_upgrade_e = get_first<SINGLE_PersistentUpgradesMenuUI>(r);
    const auto options_ui_e = get_first<SINGLE_OptionsMenuState>(r);
    SINGLE_PersistentUpgradesMenuUI* upgrade_c = nullptr;
    SINGLE_OptionsMenuState* options_ui_c = nullptr;
    if (menu_upgrade_e != entt::null)
      upgrade_c = &r.get<SINGLE_PersistentUpgradesMenuUI>(menu_upgrade_e);
    if (options_ui_e != entt::null)
      options_ui_c = &r.get<SINGLE_OptionsMenuState>(options_ui_e);
    if (upgrade_c && upgrade_c->open)
      return;
    if (options_ui_c && options_ui_c->open)
      return;
  }

  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, ui_e, ui_c);
  const auto& ri = SINGLE_RendererInfo::instance;
  auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  const auto& ui_scale = get_first_component<SINGLE_UIScaling>(r);

  //
  // Clear the handles that have joined this frame
  // Reason: prevent clicking join, and then because
  // "get_button_down" would be true again,
  // the "play" button would immediately be clicked,
  // which I doubt is the users intention
  //
  ui_c.handles_joined_this_frame.clear();

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const float pos_padding_x = -8.0f * ui_scale.scaling;
  const float pos_padding_y = 0.0f * ui_scale.scaling;
  const float ui_pos_x = viewport_pos.x + (ri.viewport_size_render_at.x * (12 / 12.0f)) + pos_padding_x;
  const float ui_pos_y = viewport_pos.y + (ri.viewport_size_render_at.y * (5 / 12.0f)) + pos_padding_y;
  const auto ui_pos = ImVec2(ui_pos_x, ui_pos_y);
  ImGui::SetNextWindowPos(ui_pos, ImGuiCond_Always, ImVec2(1.0f, 0.5f));

  // Get state for UI.
  const int active = steam_c.n_active;
  const auto data = ui_c;
  const float padding_between_player_rows = 4.0f;
  const float total_padding_y = (3 * padding_between_player_rows);

  const float total_size_x = 260 * ui_scale.scaling;
  const float total_size_y = 4 * 100 * ui_scale.scaling + total_padding_y;
  ImGui::SetNextWindowSize({ total_size_x, total_size_y }, ImGuiCond_Always);

  // There's a bug where the action handles aren't non-zero until a config is done loading. Soon config
  // information will be available immediately. Until then try to init as long as the handles are invalid.
  const auto& digital_action_handles = steam_c.digital_action_handles;
  if (digital_action_handles[(int)DA::Game_Up] == 0) {

    // poll: do this every X
    const float poll_interval = 1.0f;
    if (timer_inc > poll_interval) {
      init_steam_input_actions(r);
      timer_inc -= poll_interval;
    }

    timer += dt;
    timer_inc += dt;

    bool steaminput_enabled = SteamInput();

    imgui_begin("WaitingForControllerUI", ImGuiWindowFlags_NoInputs);
    auto txt = std::format("Detecting Controllers... {:0.1f}", timer);
    txt += "\n(Requires SteamInput Enabled)";

    ImGui::Text("%s", txt.c_str());
    ImGui::End();

    return;
  }

  // ImGui::Text("Connected Controllers: %i", steam_c.n_active);
  // ImGui::Text("Assigned Controllers: %i", (int)non_zero_handles(ui_c.handles).size());
  // ImGui::Separator();

  static std::unordered_map<InputHandle_t, std::string> join_key_map;

  for (int i = 0; i < steam_c.n_active; i++) {
    const InputHandle_t handle = steam_c.handles[i];
    if (handle == 0)
      continue;

    // ActionSet
    const auto& actionset_handles = steam_c.action_set_handles;
    const auto as = actionset_handles[(int)AS::ActionSet_GameControls];

    // DigitalAction
    const auto& digital_action_handles = steam_c.digital_action_handles;
    const auto h = digital_action_handles[(int)DA::Game_South];

    EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS];
    const auto n_origins = SteamInput()->GetDigitalActionOrigins(handle, as, h, origins);
    if (n_origins > 0) {
      // use the first origin keyname
      EInputActionOrigin origin = origins[0];
      const char* keyname = SteamInput()->GetStringForActionOrigin(origin);
      join_key_map[handle] = keyname;
    }

    if (!join_key_map.contains(handle))
      join_key_map[handle] = "Loading...";

    // Just keep assigning controllers
    assign_handle_to_ui(ui_c, handle);

    // auto b_join = controller_button_down(steam_c, handle, DA::Game_Select);
    // if (b_join) {
    //   assign_handle_to_ui(ui_c, handle);
    //   continue;
    // }

    // auto b_leave = controller_button_down(steam_c, handle, DA::Game_Cancel);
    // if (b_leave) {
    //   unassign_handle_from_ui(ui_c, handle);
    //   continue;
    // }
  }

  // i.e. "waiting to assign"
  const auto free_controllers = connected_but_not_joined_controllers(steam_c, ui_c);
  int next_free_controller = 0;

  for (int i = 0; i < 4; i++) {
    // ImGui::Text("%s", ("P"s + std::to_string(i)).c_str());

    const auto handle = ui_c.handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(ui_c, handle);

    if (joined && !connected) {
      // ImGui::SameLine();
      // ImGui::Text("Disconnected!");
      continue;
    }

    if (joined && connected) {
      // ImGui::SameLine();
      // ImGui::Text("Connected. %zu", ui_c.handles[i]);
      // ImGui::Text("Connected.");
      continue;
    }

    bool all_assigned = next_free_controller >= (int)free_controllers.size();
    if (free_controllers.empty() || all_assigned) {
      // ImGui::SameLine();
      // ImGui::Text("No controller.");
      continue; // no more free controllers
    }

    const auto unassigned_handle = free_controllers[next_free_controller++];
    const auto unassigned_handle_joinkey = join_key_map[unassigned_handle];
    const auto str = std::format("Press '{}' to join.", unassigned_handle_joinkey);

    // ImGui::SameLine();
    // ImGui::Text("%s", str.c_str());
  }

  // ImGui::End();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  imgui_begin("controllerUI", ImGuiWindowFlags_NoInputs);

  const ImVec2 window_pos = ImGui::GetWindowPos();
  const ImVec2 window_size = ImGui::GetWindowSize();

  const float non_padded_y = (window_size.y - total_padding_y) / 4.0;
  const auto player_ui_w = window_size.x;
  const auto player_ui_h = non_padded_y;
  auto player_ui_tl = ImVec2{ window_pos.x, window_pos.y };
  auto player_ui_br = ImVec2{ window_pos.x + player_ui_w, window_pos.y + player_ui_h };

  for (int i = 0; i < ui_c.players; i++) {
    const auto handle = data.handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(ui_c, handle);

    ControllerState ui_state = ControllerState::NOT_CONNECTED;
    if (joined && connected)
      ui_state = ControllerState::CONNECTED;
    if (joined && !connected)
      ui_state = ControllerState::DISCONNECTED;
    if (!joined && !connected)
      ui_state = ControllerState::NOT_CONNECTED;

#if defined(_DEBUG)
// Force boxes in to specific states
// if (i == 1)
//   draw_player_ui_box(
//     r, player_ui_tl, { player_ui_w, player_ui_h }, i, steam_c, handle, ControllerState::DISCONNECTED, dt);
// else if (i == 2)
//   draw_player_ui_box(
//     r, player_ui_tl, { player_ui_w, player_ui_h }, i, steam_c, handle, ControllerState::NOT_CONNECTED, dt);
// else
#endif
    draw_player_ui_box(r, player_ui_tl, { player_ui_w, player_ui_h }, i, steam_c, handle, ui_state, dt);

    // move vertically
    player_ui_tl.y += player_ui_h + padding_between_player_rows;
    player_ui_br.y += player_ui_h + padding_between_player_rows;
  }

  // ImGui::Text("Connected but no inputs? Please try replugging controller.");
  ImGui::End();

  const auto help_window_pos = ImVec2{ ui_pos.x - total_size_x, ui_pos.y + 0.5f * total_size_y };
  const auto help_window_size = ImVec2{ total_size_x, ri.viewport_size_render_at.y - help_window_pos.y };
  ImGui::SetNextWindowPos(help_window_pos, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(help_window_size, ImGuiCond_Always);

  imgui_begin("PlayerNoInputSuggestions");

  ImGui::NewLine();
  ImGui::SeparatorText("Help! Connected but no input!");
  ImGui::Text("- Check SteamInput is enabled.");
  ImGui::Text("- Unplug/Replug Controller.");
  ImGui::Text("- Check with a HTML5 gamepad tester.");
  ImGui::Text("- If buttons show '...'");
  ImGui::Text("  => Assign inputs via Steam Overlay.");
  ImGui::Text("  => Potentially restart the game.");

  if (ImGui::Button("Reset")) {
    for (int i = 0; i < ui_c.players; i++)
      ui_c.handles[i] = 0;
    ui_c.handles_joined_this_frame.clear();
  }

  ImGui::End();

  ImGui::PopStyleVar(2);
}

} // namespace game2d