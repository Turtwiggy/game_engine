#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "ui_popup_controller_disconnected_components.hpp"
#include "ui_popup_controller_disconnected_system.hpp"

namespace game2d {

enum class UIControllerState
{
  CONNECTED,
  CONNECTED_BUT_WAITING_FOR_INPUT,
  DISCONNECTED,
  NOT_CONNECTED,
};

void
figure_out_if_controllers_disconnected(entt::registry& r, SINGLE_DisconnectedControllerUI& ui_c)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, ui_steam_e, ui_steam_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c);

  for (int i = 0; i < 4; i++) {
    const auto handle = ui_steam_c.handles[i];
    const bool connected = handle_is_connected(steam_c, handle);
    const bool joined = handle_is_joined(ui_steam_c, handle);
    const bool is_disconnected = joined && !connected;
    const bool is_connected = joined && connected;
    if (!is_disconnected)
      continue;
    const auto it = std::find(ui_c.handle_disconnected.begin(), ui_c.handle_disconnected.end(), handle);
    if (it == ui_c.handle_disconnected.end())
      ui_c.handle_disconnected.push_back(handle);
  }
};

void
update_ui_popup_controller_disconnected_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_CurrentScene, r, scene_e, scene_c);
  GET_FIRST_OR_RETURN(SINGLE_DisconnectedControllerUI, r, ui_e, ui_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, ui_steam_e, ui_steam_c);
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c);

  const float x_align_0 = 0.33f;
  const float x_align_1 = 0.5f;

  // the only scene not to show this ui is in the main menu
  if (scene_c.s == Scene::menu) {
    ui_c.handle_disconnected.clear();
    return;
  }

  figure_out_if_controllers_disconnected(r, ui_c);

  ui_c.open = ui_c.handle_disconnected.size() != 0;
  if (!ui_c.open)
    return;

  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b = g_input_c.button_s;
  const bool do_act = std::find(b.begin(), b.end(), ActionStateEnum::DOWN) != b.end();

  // For all the disconnected handles, sample input
  for (const InputHandle_t handle : ui_c.handle_disconnected) {
    // any input
    bool input = false;
    for (int i = 0; i < (int)DigitalAction::count; i++) {
      const auto act = static_cast<DigitalAction>(i);
      input |= controller_button_held(steam_c, handle, act);
    }
    const auto lx = controller_axis(r, handle, AA::LAnalogControls);
    const auto rx = controller_axis(r, handle, AA::RAnalogControls);
    input |= lx.x != 0.0f || lx.y != 0.0f || rx.x != 0.0f || rx.y != 0.0f;

    // A handle that disconnected just pressed some input.
    // note: this immediately closes the menu.
    if (input)
      std::erase(ui_c.handle_disconnected, handle);
  }

  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto header_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_20 : FontSize::TEXT_SIZE_20_SCALED;
  const auto header_font_size = (float)header_font_enum;
  auto* header_font = get_inter_font(r, header_font_enum);
  const auto text_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto text_font_size = (float)text_font_enum;
  auto* text_font = get_inter_font(r, text_font_enum);
  const auto TEXT_SIZE = text_font->CalcTextSizeA(text_font_size, FLT_MAX, -1, "A");

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, 0 });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto window_pos = ImVec2{ ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f };
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2{ 0.5f, 0.5f });

  const auto window_size = glm::vec2{ 300 * font_scale, 175 * font_scale };
  ImGui::SetNextWindowSize({ window_size.x, window_size.y }, ImGuiCond_Always);

  ImGui::Begin("DisconnectedUI", NULL, flags);
  {
    const auto ui_tl = ImGui::GetWindowPos();
    const auto ui_wh = ImGui::GetWindowSize();
    const auto ui_br = ImVec2{ ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };
    auto text_y = ui_tl.y;

    auto* draw_list = ImGui::GetWindowDrawList();
    const auto my_window_bg_col = hex_to_srgb("#21242B");
    const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
    const auto my_window_border_col = hex_to_srgb("#FFFFFF");
    const auto im_window_border_col = convert_my_to_im(my_window_bg_col);
    const auto rounding = 4.0f;
    const auto thickness = 2.0f;
    const auto rect_flags = ImDrawFlags_RoundCornersAll;
    draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
    draw_list->AddRect(ui_tl, ui_br, IM_COL32(255, 255, 255, 255), rounding, rect_flags, thickness);

    text_y += 4.0f; // add some padding

    const auto header = std::string("Controller Disconnected. >:(");
    const auto header_size = header_font->CalcTextSizeA(header_font_size, FLT_MAX, -1, header.c_str());
    const auto header_pos = ImVec2{ ui_tl.x + ui_wh.x * 0.5f - header_size.x * 0.5f, ui_tl.y + 4.0f };
    draw_list->AddText(header_font, header_font_size, header_pos, IM_COL32(255, 255, 255, 255), header.c_str());

    text_y += header_size.y;
    text_y += 10.0f; // add some padding

    for (int i = 0; i < 4; i++) {
      const auto handle = ui_steam_c.handles[i];
      const bool connected = handle_is_connected(steam_c, handle);
      const bool joined = handle_is_joined(ui_steam_c, handle);

      const bool is_disconnected = joined && !connected;
      const bool is_connected = joined && connected;
      const bool not_connected = !joined;

      UIControllerState state = UIControllerState::NOT_CONNECTED;

      if (not_connected)
        state = UIControllerState::NOT_CONNECTED;
      else if (is_disconnected)
        state = UIControllerState::DISCONNECTED;
      else if (is_connected) {
        auto it = std::find(ui_c.handle_disconnected.begin(), ui_c.handle_disconnected.end(), handle);
        if (it == ui_c.handle_disconnected.end())
          state = UIControllerState::CONNECTED;
        else
          state = UIControllerState::CONNECTED_BUT_WAITING_FOR_INPUT;
      }

#if defined(_DEBUG)
      // if (i == 0)
      //   state = UIControllerState::CONNECTED;
      // if (i == 1)
      //   state = UIControllerState::DISCONNECTED;
      // if (i == 2)
      //   state = UIControllerState::CONNECTED_BUT_WAITING_FOR_INPUT;
      // if (i == 3)
      //   state = UIControllerState::NOT_CONNECTED;
#endif

      const auto text_l_pos = ImVec2{ ui_tl.x + (ui_wh.x * x_align_0), text_y };
      const auto text_r_pos = ImVec2{ ui_tl.x + (ui_wh.x * x_align_1), text_y };

      const auto my_con_col = hex_to_srgb("#1AFF00");
      const auto my_con_wait_input_col = hex_to_srgb("#E8DA58");
      const auto my_dc_col = hex_to_srgb("#C78B1A");
      const auto my_not_con_col = hex_to_srgb("#736767");

      const auto im_con_col = convert_my_to_im(my_con_col);
      const auto im_con_wait_input_col = convert_my_to_im(my_con_wait_input_col);
      const auto im_dc_col = convert_my_to_im(my_dc_col);
      const auto im_not_con_col = convert_my_to_im(my_not_con_col);
      const auto key_col = joined ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 100);

      std::string key = std::format("P{}", i + 1);
      std::string val = "N/A";
      ImU32 col = im_not_con_col;

      if (state == UIControllerState::CONNECTED) {
        val = "CON";
        col = im_con_col;
      } else if (state == UIControllerState::CONNECTED_BUT_WAITING_FOR_INPUT) {
        val = "CON - Waiting for Input";
        col = im_con_wait_input_col;
      } else if (state == UIControllerState::DISCONNECTED) {
        val = "DC";
        col = im_dc_col;
      } else if (state == UIControllerState::NOT_CONNECTED) {
        val = "N/A";
        col = im_not_con_col;
      } else
        throw std::runtime_error("Unknown UI state");

      // player: 1
      draw_list->AddText(text_font, text_font_size, text_l_pos, key_col, key.c_str());

      // player: 1 controller state
      draw_list->AddText(text_font, text_font_size, text_r_pos, col, val.c_str());

      text_y += TEXT_SIZE.y;
    }

    // TODO: If all joined are connected, show the next button.
    const bool button_active = ui_c.handle_disconnected.size() == 0;
    const ImVec2 button_size = { 160.0f, (TEXT_SIZE.y * 2.0f) + 2.0f };

    int row_idx = 0;
    int col_idx = 0;
    auto a_def = SelectableButtonDef{
      .label = "Resume",
      .size = button_size,
      .input = do_act && button_active,
      .my_row_index = 0,
      .my_col_index = 0,
      .ui_row_index = row_idx,
      .ui_col_index = col_idx,
      .ui_col_active = button_active,

      .font = text_font,
    };

    const auto cursor_pos = ImVec2{
      ui_tl.x + ui_wh.x * 0.5f - button_size.x * 0.5f, // center
      ui_tl.y + ui_wh.y - 2.0f * button_size.y         // bottom
    };
    ImGui::SetCursorScreenPos(cursor_pos);

    if (selectable_button(r, a_def)) {
      // resume
      ui_c.handle_disconnected.clear();
      ui_c.open = false;
    }
  }

  ImGui::End();
  ImGui::PopStyleVar(5);
}

} // namespace game2d