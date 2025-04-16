#include "pch.hpp"

#include "ui_popup_options_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"

namespace game2d {
using namespace std::literals;

void
back_to_main_menu(entt::registry& r, SINGLE_OptionsMenuState& ui_c)
{
  SDL_Log("Closing Options Menu");
  ui_c.open = false;
  ui_c.one_frame_buffer = true;
  ui_c.state.current_row_index = 0;

  // save your changes
  savefile_save_disk(r);

  create_empty<RequestToShowMainMenu>(r);
}

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
  ui_c.state.rows.push_back(RowState{ .col_name = "Back", .action = [&r, &ui_c]() { back_to_main_menu(r, ui_c); } });
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
  const bool do_back =
    std::find(ui_c.state.actions.begin(), ui_c.state.actions.end(), UIAction::BACK) != ui_c.state.actions.end();

  if (do_back) {
    back_to_main_menu(r, ui_c);
    return;
  }

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoBackground;

  // const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  // const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto font_enum = FontSize::TEXT_SIZE_16;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);

  // idx: 3 should be fingerpaint, idx: 4 should be fingerpaint scaled.
  // auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[font_scale == 1.0f ? 3 : 4];
  // const auto header_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto header_font_enum = FontSize::TEXT_SIZE_16;
  auto* header_font = get_inter_font(r, header_font_enum);

  ImGui::PushFont(font);

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 6 });

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + viewport_wh_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  // const auto window_size = glm::vec2{ 300 * font_scale, 175 * font_scale };
  // imgui_draw_vec2("window_size", window_size);
  // ImGui::SetNextWindowSize({ window_size.x, window_size.y }, ImGuiCond_Always);

  ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  ImGui::Begin("Options Menu", NULL, flags);

  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 window_br = { window_tl.x + window_wh.x, window_tl.y + window_wh.y };

  const auto my_separator_col = hex_to_srgb("#7d8488");
  const auto my_inactive_col = hex_to_srgb("#737a7e");
  const auto my_window_bg_col = hex_to_srgb("#0c1116");
  const auto im_separator_col = convert_my_to_im_vec(my_separator_col);
  const auto im_inactive_col = convert_my_to_im_vec(my_inactive_col);
  const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
  const auto rounding = 12.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, rounding);
  // draw_list->AddRect(window_tl, window_br, IM_COL32(255, 255, 255, 255), rounding, rect_flags, thickness);

  const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };
  const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const float padding_x = 10;

  // centered options header
  ImGui::PushFont(header_font);
  const auto header_text = std::string("OPTIONS");
  const auto header_pos = center_text(header_font, header_text, { window_wh.x * 0.5f, 0 });
  ImGui::SetCursorPosX(header_pos.x);
  ImGui::SetCursorPosY(-1 * header_pos.y);
  ImGui::TextColored(white_col, "%s", header_text.c_str());
  ImGui::PopFont();

  ImGui::PushStyleColor(ImGuiCol_Separator, im_separator_col);
  ImGui::Separator();
  ImGui::PopStyleColor();

  for (int i = 0; i < (int)(ui_c.state.rows.size()); i++) {
    auto& row = ui_c.state.rows[i];

    // center the"back" button text
    auto but_size = button_size;
    float pad_x = padding_x;
    bool text_centered = false;
    if (to_lower(row.col_name).find("back") != std::string::npos) {
      text_centered = true;
      pad_x = 0;
      but_size.x = window_wh.x;
    }

    int col_idx = 0;
    auto a_def = SelectableButtonDef{
      .label = to_upper(row.col_name),
      .size = but_size,
      .input = do_act,
      .my_row_index = i,
      .my_col_index = 0, // one column
      .ui_row_index = ui_c.state.current_row_index,
      .ui_col_index = col_idx, // one column
      .ui_col_active = true,   // one column

      // could replace both .text_X with .text_pivot
      .text_centered = text_centered,
      .text_offset = { pad_x, 0 },

      .font = font,

      // hide the buttons
      .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    // add seperators for categories.
    const auto enum_val = magic_enum::enum_cast<GAME_OPTIONS>(i).value();

    // first audio option
    if (enum_val == GAME_OPTIONS::AUDIO_MASTER_VOLUME) {
      const auto h_txt = "- AUDIO -";
      const auto h_pos = center_text(font, h_txt, { window_wh.x * 0.5f, 0 });
      ImGui::SetCursorPosX(h_pos.x);
      ImGui::TextColored(white_col, h_txt);
    }

    // first video option
    if (enum_val == GAME_OPTIONS::VIDEO_SCREEN_MODE) {
      const auto h_txt = "- VIDEO -";
      const auto h_pos = center_text(font, h_txt, { window_wh.x * 0.5f, 0 });
      ImGui::SetCursorPosX(h_pos.x);
      ImGui::TextColored(white_col, h_txt);
    }

    // last option
    // if (i == (int)(GAME_OPTIONS::count)) {
    //   const auto h_txt = "- SYSTEM -";
    //   const auto h_pos = center_text(font, h_txt, { window_wh.x * 0.5f, 0 });
    //   ImGui::SetCursorPosX(h_pos.x);
    //   ImGui::TextColored(white_col, h_txt);
    // }

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

        ImGui::SetCursorPos({ padding_x, ImGui::GetCursorPos().y });
        ImGui::TextColored(im_inactive_col, "RESOLUTION (auto)");

        ImGui::SameLine();
        ImGui::SetCursorScreenPos({ window_tl.x + window_wh.x * 0.50f, ImGui::GetCursorScreenPos().y });
        ImGui::TextColored(im_inactive_col, "%i %i", ri_c.viewport_size_render_at.x, ri_c.viewport_size_render_at.y);
        continue;
      }
    }

    if (selectable_button(r, a_def))
      row.action();

    std::shared_ptr<IOption> option = get_option(r, enum_val);
    if (option == nullptr)
      continue; // option not impl?

    // Update option...
    if (active && h_value_changed) {
      auto& h_value = row.col_index;
      option->update(app, r, h_value);
    }

    {
      // Display option value to user...
      // ImGui::SameLine(window_wh.x * 0.6f);
      // ImGui::Text("%s", option->display_val().c_str());

      const auto display_slider = [&r, &app, &row, window_wh, padding_x](auto& o, auto label) {
        auto& data = o->data;

        ImGui::SameLine(window_wh.x * 0.5f);
        ImGui::Text("%s", o->display_val().c_str());

        float slider_x = 0.6f; // 0-1
        ImGui::SameLine(window_wh.x * slider_x);
        ImGui::SetNextItemWidth((window_wh.x * (1.0f - slider_x)) - padding_x);
        if (ImGui::SliderFloat(label, &data.value, 0.0f, 1.0f, "", 0)) {
          int tmp = engine::scale(data.value, 0.0f, 1.0f, 0, 10);
          o->update(app, r, tmp);
          row.col_index = tmp; // update the row col_index
        }
      };

      const auto display_button = [&r, &app, &row, window_wh, padding_x](auto& o, auto label) {
        auto& data = o->data;
        ImGui::SameLine(window_wh.x * 0.75f);
        if (ImGui::Checkbox(label, &data.enabled)) {
          int tmp = (int)data.enabled;
          o->update(app, r, tmp);
        }
      };

      const auto display_options = [&r, &app, &row, window_wh](auto& o, int cur_opt, auto l_tag, auto r_tag) {
        auto& data = o->data;
        ImGui::SameLine(window_wh.x * 0.5f);

        float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
        if (ImGui::ArrowButton(l_tag, ImGuiDir_Left)) {
          int new_dm = (int)cur_opt - 1;
          o->update(app, r, new_dm);
          row.col_index = new_dm;
        }

        ImGui::SameLine(0.0f, spacing);
        const auto display_txt = o->display_val();
        ImGui::Text("%s", display_txt.c_str());

        ImGui::SameLine(0.0f, spacing);
        if (ImGui::ArrowButton(r_tag, ImGuiDir_Right)) {
          int new_dm = (int)cur_opt + 1;
          o->update(app, r, new_dm);
          row.col_index = new_dm;
        }
      };

      if (auto* o = dynamic_cast<Option_AudioMasterVolume*>(option.get())) {
        display_slider(o, "##mastervol");
      }
      if (auto* o = dynamic_cast<Option_AudioMusicVolume*>(option.get())) {
        display_slider(o, "##musicvol");
      }
      if (auto* o = dynamic_cast<Option_AudioSFXVolume*>(option.get())) {
        display_slider(o, "##sfxvol");
      }
      if (auto* o = dynamic_cast<Option_VideoScreenMode*>(option.get())) {
        display_options(o, o->get_hindex(r), "##vsm-l", "##vsm-r");
      }
      if (auto* o = dynamic_cast<Option_VideoResolution*>(option.get())) {
        display_options(o, o->get_hindex(r), "##res-l", "##res-r");
      }
      if (auto* o = dynamic_cast<Option_VideoVsync*>(option.get())) {
        display_button(o, "##vsync");
      }
    }
  }

  ImGui::NewLine(); // pad the last row
  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar();
}

} // namespace game2d