#include "pch.hpp"

#include "ui_popup_options_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/core_io/io_helpers.hpp"
#include "modules/core_options/options_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_components.hpp"

namespace game2d {

std::shared_ptr<IOption>
get_option(entt::registry& r, const GAME_OPTIONS o)
{
  const auto& options_c = get_first_component<SINGLE_GameOptions>(r);
  const auto& opts = options_c.options;

  const auto find_by_option = [o](std::shared_ptr<IOption> option) { return option->option == o; };
  const auto it = std::find_if(opts.begin(), opts.end(), find_by_option);
  if (it == opts.end())
    return nullptr;

  return (*it);
};

std::string
game_option_enum_to_display_string(entt::registry& r, const GAME_OPTIONS o)
{
  auto opt_it = get_option(r, o);
  if (opt_it != nullptr)
    return opt_it->display_str;
  return "not impl";
  throw std::runtime_error("GAME_OPTION not handled.");
  return "not impl";
}

int
game_option_enum_load_from_prefs(entt::registry& r, const GAME_OPTIONS o)
{
  auto opt_it = get_option(r, o);
  if (opt_it == nullptr)
    return 0;
  return opt_it->get_hindex(r);
}

void
init_menu(entt::registry& r, SINGLE_OptionsMenuState& ui_c)
{
  for (int i = 0; i < (int)GAME_OPTIONS::count; i++) {
    const auto enum_val = magic_enum::enum_cast<GAME_OPTIONS>(i).value();
    const auto enum_str = std::string(magic_enum::enum_name(enum_val));

    ui_c.state.rows.push_back(RowState{
      .col_name = game_option_enum_to_display_string(r, enum_val),
      .col_index = game_option_enum_load_from_prefs(r, enum_val),
      .action = []() {},
    });
  }

  // go back to whence you came!
  ui_c.state.rows.push_back(RowState{ .col_name = "Back", .action = [&r, &ui_c]() {
                                       SDL_Log("Closing Options Menu");
                                       ui_c.open = false;
                                       ui_c.one_frame_buffer = true;
                                       ui_c.state.current_row_index = 0;

                                       // save your changes
                                       savefile_save_disk(r);

                                       create_empty<RequestToShowMainMenu>(r);
                                     } });
}

void
update_ui_popup_options_system(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  auto& ui_c = gesert_component<SINGLE_OptionsMenuState>(r);
  const auto& ri_c = get_first_component<SINGLE_RendererInfo>(r);

  process_requests<RequestToShowOptionsMenu>(r, [&ui_c](const auto& req) {
    ui_c.one_frame_buffer = true;
    ui_c.open = true;
  });

  if (!ui_c.open)
    return;

  if (!ui_c.init) {
    init_menu(r, ui_c);
    ui_c.init = true;
  }

  if (ui_c.one_frame_buffer) {
    ui_c.one_frame_buffer = false;
    return;
  }

  process_input_for_ui_all_handles(r, ui_c.state);

  const bool do_act =
    std::find(ui_c.state.actions.begin(), ui_c.state.actions.end(), UIAction::SELECT) != ui_c.state.actions.end();

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 2 });

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + viewport_wh_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));

  const auto window_size = glm::vec2{ 300 * font_scale, 175 * font_scale };
  // imgui_draw_vec2("window_size", window_size);
  ImGui::SetNextWindowSize({ window_size.x, window_size.y }, ImGuiCond_Always);

  ImGui::Begin("Options Menu", NULL, flags);

  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 window_br = { window_tl.x + window_wh.x, window_tl.y + window_wh.y };

  const auto my_window_bg_col = hex_to_srgb("#21242B");
  const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
  const auto my_window_border_col = hex_to_srgb("#FFFFFF");
  const auto im_window_border_col = convert_my_to_im(my_window_bg_col);
  const auto header_x_padding = 10.0f;
  const auto rounding = 4.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, rounding);
  draw_list->AddRect(window_tl, window_br, IM_COL32(255, 255, 255, 255), rounding, rect_flags, thickness);

  const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };

  for (int i = 0; i < (int)(ui_c.state.rows.size()); i++) {
    auto& row = ui_c.state.rows[i];

    int col_idx = 0;
    auto a_def = SelectableButtonDef{
      .label = row.col_name,
      .size = button_size,
      .input = do_act,
      .my_row_index = i,
      .my_col_index = 0, // one column
      .ui_row_index = ui_c.state.current_row_index,
      .ui_col_index = col_idx, // one column
      .ui_col_active = true,   // one column

      // could replace both .text_X with .text_pivot
      .text_centered = false,
      .text_offset = { window_wh.x * 0.25f, 0 },

      .font = font,

      // hide the buttons
      .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    // Hack: add seperators for categories.
    const auto enum_val = magic_enum::enum_cast<GAME_OPTIONS>(i).value();

    // first audio option
    if (enum_val == GAME_OPTIONS::AUDIO_MASTER_VOLUME) {
      // ImGui::Text("Keyboard: use arrow keys (wip)");
      // ImGui::Text("Controller: use dpad");
      ImGui::SetCursorPosX(header_x_padding);
      ImGui::Text("Audio");
    }

    // first video option
    if (enum_val == GAME_OPTIONS::VIDEO_SCREEN_MODE) {
      ImGui::SetCursorPosX(header_x_padding);
      ImGui::Text("Video");
    }

    // last option
    if (i == (int)(GAME_OPTIONS::count)) {
      ImGui::SetCursorPosX(header_x_padding);
      ImGui::Text("Menu");
    }

    const auto& acts = ui_c.state.actions;
    const auto v_value_changed = std::find(acts.begin(), acts.end(), UIAction::V_VALUE_CHANGED) != acts.end();
    const auto h_value_changed = std::find(acts.begin(), acts.end(), UIAction::H_VALUE_CHANGED) != acts.end();
    const bool active = i == ui_c.state.current_row_index;

    // if in fullscreen_borderless, hide resolution option.
    if (enum_val == GAME_OPTIONS::VIDEO_RESOLUTION) {

      const auto dm = app.window.get_displaymode();
      if (dm == engine::DisplayMode::fullscreen_borderless) {

        // Skip this option in the select menu
        if (v_value_changed && active && enum_val == GAME_OPTIONS::VIDEO_RESOLUTION) {
          const auto v_value_changed_u = std::find(acts.begin(), acts.end(), UIAction::V_VALUE_CHANGED_UP) != acts.end();
          const auto v_value_changed_d = std::find(acts.begin(), acts.end(), UIAction::V_VALUE_CHANGED_DOWN) != acts.end();
          if (v_value_changed_u)
            ui_c.state.current_row_index++;
          if (v_value_changed_d)
            ui_c.state.current_row_index--;
        }

        const auto inactive_col = ImVec4(1.0f, 1.0f, 1.0f, (100 / 255.0f));
        ImGui::SetCursorScreenPos({ window_tl.x + window_wh.x * 0.25f, ImGui::GetCursorScreenPos().y });
        ImGui::TextColored(inactive_col, "Resolution (auto)");

        ImGui::SameLine();
        ImGui::SetCursorScreenPos({ window_tl.x + window_wh.x * 0.60f, ImGui::GetCursorScreenPos().y });
        ImGui::TextColored(inactive_col, "%i %i", ri_c.viewport_size_render_at.x, ri_c.viewport_size_render_at.y);
        continue;
      }
    }

    if (selectable_button(r, a_def))
      row.action();

    const auto& option = get_option(r, enum_val);
    if (option == nullptr)
      continue; // option not impl?

    // Update option...
    if (active && h_value_changed) {
      auto& h_value = row.col_index;
      option->update(app, r, h_value);
    }

    // Display option value to user...
    ImGui::SameLine(window_wh.x * 0.6f);
    ImGui::Text("%s", option->display_val().c_str());
  }

  ImGui::NewLine(); // pad the last row
  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar(5);
}

} // namespace game2d