#include "pch.hpp"

#include "ui_popup_options_helpers.hpp"
#include "ui_popup_options_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/maths.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "resources/data.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_popup_options_system(engine::SINGLE_Application& app, entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri_c = SINGLE_RendererInfo::instance;
  auto& ui_c = gesert_component<SINGLE_OptionsMenuState>(r);

  ui_c.update<RequestToShowOptionsMenu>(r);
  if (!ui_c.open)
    return;
  if (!ui_c.init)
    ui_c.do_init(r);

  // input
  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const auto& b_e = g_input_c.button_e;
  const bool do_act = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();
  const bool do_back = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();

  if (do_back) {
    back_to_main_menu(r, ui_c);
    return;
  }

  // map ui navigation action to updating the data in the options cell.
  const auto& acts = ui_c.state.actions;
  const bool v_value_changed_u = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_U) != acts.end();
  const bool v_value_changed_d = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_D) != acts.end();
  const bool h_value_changed_l = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_L) != acts.end();
  const bool h_value_changed_r = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_R) != acts.end();
  const bool v_value_changed = v_value_changed_u || v_value_changed_d;
  const bool h_value_changed = h_value_changed_l || h_value_changed_r;

  // const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  // const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto font_enum = FontSize::TEXT_SIZE_16;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  // idx: 3 should be fingerpaint, idx: 4 should be fingerpaint scaled.
  const auto header_font_enum = FontSize::TEXT_SIZE_16;
  auto* header_font = get_inter_font(r, header_font_enum);

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 6 });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 3, 6 });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);

  const auto viewport_tl = ImVec2((float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y);
  const auto viewport_wh = ImVec2(ri_c.viewport_size_render_at.x, ri_c.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + viewport_wh_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  imgui_begin("OptionsMenu");

  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 window_br = { window_tl.x + window_wh.x, window_tl.y + window_wh.y };

  const auto rounding = 12.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  // draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, rounding);

  const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };
  const float padding_x = 10;

  for (int i = 0; i < (int)ui_c.state.cells.size(); i++) {
    auto& base = ui_c.state.cells[i];
    auto* cell = dynamic_cast<OptionsCell*>(base.get());

    // which is the active cell index
    const auto cell_it = std::find(ui_c.state.cells.begin(), ui_c.state.cells.end(), ui_c.state.active);
    const auto cell_idx = static_cast<int>(cell_it - ui_c.state.cells.begin());
    const int col_idx = 0;
    const int row_idx = cell_idx;
    const bool active = base == ui_c.state.active;

    auto a_def = SelectableButtonDef{
      .display_str = to_upper(cell->name),
      .imgui_hash = "##" + cell->name,
      .size = button_size,
      .input = do_act,
      .cell = base,
      .active_cell = ui_c.state.active,

      .text_pivot = { 0.0f, 0.5f }, // center_y
      .text_offset = { padding_x, 0 },
      .font = font,

      // hide the buttons
      .active_outline_col = { 1.0f, 0.0f, 0.0f, 1.0f },
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

    // if in fullscreen_borderless, hide resolution option.
    if (enum_val == GAME_OPTIONS::VIDEO_RESOLUTION) {

      const auto dm = app.window.get_displaymode();
      if (dm == engine::DisplayMode::fullscreen_borderless) {

        // Skip this option in the select menu
        if (active && enum_val == GAME_OPTIONS::VIDEO_RESOLUTION) {
          if (v_value_changed) {
            if (v_value_changed_u)
              ui_c.state.active = ui_c.state.cells[row_idx - 1]; // prv
            else if (v_value_changed_d)
              ui_c.state.active = ui_c.state.cells[row_idx + 1]; // nxt
            else
              ui_c.state.active = ui_c.state.cells[row_idx + 1]; // force next
          }
        }

        ImGui::NewLine();
        ImGui::SameLine(padding_x);
        ImGui::TextColored(im_inactive_col, "RESOLUTION (auto)");

        ImGui::SameLine(window_wh.x * 0.5f);
        ImGui::TextColored(im_inactive_col, "%i %i", ri_c.viewport_size_render_at.x, ri_c.viewport_size_render_at.y);
        continue;
      }
    }

    if (selectable_button(r, a_def))
      cell->action();

    std::shared_ptr<IOption> option = get_option(r, enum_val);
    if (option == nullptr)
      continue; // option not impl?

    // Update option...
    if (active && h_value_changed) {
      auto& h_value = cell->value;
      if (h_value_changed_r)
        h_value++;
      if (h_value_changed_l)
        h_value--;
      option->update(app, r, h_value);
    }

    const auto display_slider = [&r, &app, &cell, window_wh, padding_x](auto& o, auto label) {
      auto& data = o->data;

      ImGui::SameLine(window_wh.x * 0.5f);
      ImGui::Text("%s", o->display_val().c_str());

      float slider_x = 0.6f; // 0-1
      ImGui::SameLine(window_wh.x * slider_x);
      ImGui::SetNextItemWidth((window_wh.x * (1.0f - slider_x)) - padding_x);
      if (ImGui::SliderFloat(label, &data.value, 0.0f, 1.0f, "", 0)) {
        int tmp = engine::scale(data.value, 0.0f, 1.0f, 0, 10);
        o->update(app, r, tmp);
        cell->value = tmp; // update the row col_index
      }
    };

    const auto display_button = [&r, &app, window_wh, padding_x](auto& o, auto label) {
      auto& data = o->data;
      ImGui::SameLine(window_wh.x * 0.75f);
      if (ImGui::Checkbox(label, &data.enabled)) {
        int tmp = (int)data.enabled;
        o->update(app, r, tmp);
      }
    };

    const auto display_options = [&r, &app, &cell, window_wh](auto& o, int cur_opt, auto l_tag, auto r_tag) {
      auto& data = o->data;
      ImGui::SameLine(window_wh.x * 0.5f);

      float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
      if (ImGui::ArrowButton(l_tag, ImGuiDir_Left)) {
        int new_dm = (int)cur_opt - 1;
        o->update(app, r, new_dm);
        cell->value = new_dm;
      }

      ImGui::SameLine(0.0f, spacing);
      const auto display_txt = o->display_val();
      ImGui::Text("%s", display_txt.c_str());

      ImGui::SameLine(0.0f, spacing);
      if (ImGui::ArrowButton(r_tag, ImGuiDir_Right)) {
        int new_dm = (int)cur_opt + 1;
        o->update(app, r, new_dm);
        cell->value = new_dm;
      }
    };

    if (auto* o = dynamic_cast<Option_AudioMasterVolume*>(option.get()))
      display_slider(o, "##mastervol");

    if (auto* o = dynamic_cast<Option_AudioMusicVolume*>(option.get()))
      display_slider(o, "##musicvol");

    if (auto* o = dynamic_cast<Option_AudioSFXVolume*>(option.get()))
      display_slider(o, "##sfxvol");

    if (auto* o = dynamic_cast<Option_VideoScreenMode*>(option.get()))
      display_options(o, o->get_hindex(r), "##vsm-l", "##vsm-r");

    if (auto* o = dynamic_cast<Option_VideoResolution*>(option.get()))
      display_options(o, o->get_hindex(r), "##res-l", "##res-r");

    if (auto* o = dynamic_cast<Option_VideoVsync*>(option.get()))
      display_button(o, "##vsync");

    if (i == ui_c.state.cells.size() - 1) {
      ImGui::PushStyleColor(ImGuiCol_Separator, im_separator_col);
      ImGui::Separator();
      ImGui::PopStyleColor();
    }
  }

  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar(6);
}

} // namespace game2d