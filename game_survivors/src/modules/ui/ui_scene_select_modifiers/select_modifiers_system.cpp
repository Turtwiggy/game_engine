#include "pch.hpp"

#include "select_modifiers_components.hpp"
#include "select_modifiers_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"

namespace game2d {

void
back(entt::registry& r)
{
  move_to_scene_start(r, Scene::menu);
};

void
update_ui_scene_select_modifiers_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& ui_scale = get_first_component<SINGLE_UIScaling>(r);
  auto& ui_c = gesert_component<SINGLE_UISelectModifiersMenuState>(r);

  if (!ui_c.init)
    ui_c.do_init(r);

  ui_c.update<RequestToShowModifierMenu>(r);
  if (!ui_c.open)
    return;

  process_input_for_ui_all_handles(r, ui_c.state);
  const bool do_act =
    std::find(ui_c.state.actions.begin(), ui_c.state.actions.end(), UIAction::SELECT) != ui_c.state.actions.end();
  const bool do_back =
    std::find(ui_c.state.actions.begin(), ui_c.state.actions.end(), UIAction::BACK) != ui_c.state.actions.end();

  if (do_back) {
    back(r);
    return;
  }

  const auto font_enum = FontSize::TEXT_SIZE_16;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  // idx: 3 should be fingerpaint, idx: 4 should be fingerpaint scaled.
  // auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[font_scale == 1.0f ? 3 : 4];
  // const auto header_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto header_font_enum = FontSize::TEXT_SIZE_16;
  auto* header_font = get_inter_font(r, header_font_enum);

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 6 });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 3, 6 });

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2((float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + viewport_wh_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  imgui_begin("ChooseModifiers");
  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 window_br = { window_tl.x + window_wh.x, window_tl.y + window_wh.y };

  // same as the options menu
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

  const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };
  const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const float padding_x = 10;

  const auto h_txt = "- GENERAL -";
  const auto h_pos = center_text(font, h_txt, { window_wh.x * 0.5f, 0 });
  ImGui::SetCursorPosX(h_pos.x);
  ImGui::TextColored(white_col, h_txt);

  for (int i = 0; i < (int)(ui_c.state.cells.size()); i++) {
    auto& cell = ui_c.state.cells[i];

    auto a_def = SelectableButtonDef{
      .label = to_upper(cell->name),
      .size = button_size,
      .input = do_act,
      .cell = cell,
      .active_cell = ui_c.state.active,

      // could replace both .text_X with .text_pivot
      .text_centered = false,
      .text_offset = { padding_x, 0 },
      .font = font,

      // hide the buttons
      .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    if (selectable_button(r, a_def))
      cell->action();

    auto option = get_modifier_option(r, MODIFIER_OPTIONS::ROCKS);
    if (option == nullptr)
      continue; // option not impl?

    // islands only modifier
    // TODO: fix this
    if (i != 0)
      continue;

    const auto& acts = ui_c.state.actions;
    const bool v_value_changed_u = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_U) != acts.end();
    const bool v_value_changed_d = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_D) != acts.end();
    const bool h_value_changed_l = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_L) != acts.end();
    const bool h_value_changed_r = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_R) != acts.end();
    const bool v_value_changed = v_value_changed_u || v_value_changed_d;
    const bool h_value_changed = h_value_changed_l || h_value_changed_r;
    const bool active = cell == ui_c.state.active;

    // Update option...
    if (active && h_value_changed) {
      auto& h_value = dynamic_cast<OptionsCell*>(cell.get())->value;
      option->update(r, h_value);
    }

    const auto enum_val = magic_enum::enum_cast<GAME_OPTIONS>(i).value();
    const auto display_button = [&r, window_wh](auto& o, bool& data, auto label) {
      ImGui::SameLine(window_wh.x * 0.75f);
      if (ImGui::Checkbox(label, &data)) {
        int tmp = (int)data;
        o->update(r, tmp);
      }
    };

    // if (option->value_type == UIValueType::BUTTON)
    if (auto* o = dynamic_cast<Option_Rocks*>(option.get()))
      display_button(o, o->populate_rocks, "##rocks");
  }

  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar(2);
}

} // namespace game2d