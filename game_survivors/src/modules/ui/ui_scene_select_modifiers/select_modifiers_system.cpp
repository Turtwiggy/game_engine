#include "pch.hpp"

#include "select_modifiers_components.hpp"
#include "select_modifiers_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"

namespace game2d {

void
back(entt::registry& r)
{
  move_to_scene_start(r, Scene::menu);
}

void
next(entt::registry& r)
{
  move_to_scene_start(r, Scene::select_ships);
}

void
init(entt::registry& r, SINGLE_UISelectModifiersMenuState& ui_c)
{
  for (int i = 0; i < (int)MODIFIER_OPTIONS::count; i++) {
    const auto enum_val = magic_enum::enum_cast<MODIFIER_OPTIONS>(i).value();
    const auto enum_str = std::string(magic_enum::enum_name(enum_val));

    ui_c.state.rows.push_back(RowState{
      .col_name = modifier_option_enum_to_display_string(r, enum_val),
      .col_index = 0,
      .action = []() {},
    });
  }

  ui_c.state.rows.push_back(RowState{ .col_name = "Next", .action = [&r, &ui_c]() { next(r); } });
  ui_c.state.rows.push_back(RowState{ .col_name = "Back", .action = [&r, &ui_c]() { back(r); } });
  ui_c.init = true;
};

void
update_ui_scene_select_modifiers_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& ui_scale = get_first_component<SINGLE_UIData>(r);
  auto& ui_c = gesert_component<SINGLE_UISelectModifiersMenuState>(r);

  process_requests<RequestToShowModifierMenu>(r, [&ui_c](const auto& req) {
    ui_c.one_frame_buffer = true;
    ui_c.open = true;
  });
  if (!ui_c.open)
    return;
  if (!ui_c.init) {
    init(r, ui_c);
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
    back(r);
    return;
  }

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoNav;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto font_enum = FontSize::TEXT_SIZE_16;
  const auto font_size = (float)font_enum;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);
  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + viewport_wh_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSizeConstraints({ 400, 200 }, { 1000, 1000 });

  ImGui::Begin("Something", NULL, flags);
  auto* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 window_tl = ImGui::GetWindowPos();
  const ImVec2 window_wh = ImGui::GetWindowSize();
  const ImVec2 window_br = { window_tl.x + window_wh.x, window_tl.y + window_wh.y };

  // same as the options menu
  const auto rounding = 12.0f;
  const auto my_window_bg_col = hex_to_srgb("#0c1116");
  const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
  draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, rounding);

  ImGui::Text("Gameplay Modifiers");

  const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };
  const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const float padding_x = 10;

  for (int i = 0; i < (int)(ui_c.state.rows.size() - 2); i++) {
    auto& row = ui_c.state.rows[i];

    int col_idx = 0;
    auto a_def = SelectableButtonDef{
      .label = to_upper(row.col_name),
      .size = button_size,
      .input = do_act,
      .my_row_index = i,
      .my_col_index = 0, // one column
      .ui_row_index = ui_c.state.current_row_index,
      .ui_col_index = col_idx, // one column
      .ui_col_active = true,   // one column

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
      row.action();

    auto option = get_modifier_option(r, MODIFIER_OPTIONS::ROCKS);
    if (option == nullptr)
      continue; // option not impl?

    const auto& acts = ui_c.state.actions;
    const auto v_value_changed = std::find(acts.begin(), acts.end(), UIAction::V_VALUE_CHANGED) != acts.end();
    const auto h_value_changed = std::find(acts.begin(), acts.end(), UIAction::H_VALUE_CHANGED) != acts.end();
    const bool active = i == ui_c.state.current_row_index;

    // Update option...
    if (active && h_value_changed) {
      auto& h_value = row.col_index;
      option->update(r, h_value);
    }

    const auto enum_val = magic_enum::enum_cast<GAME_OPTIONS>(i).value();
    const auto display_button = [&r, &row, window_wh, padding_x](auto& o, bool& data, auto label) {
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

  // Display the last two rows; the next and the back button.
  for (int i = (ui_c.state.rows.size() - 2); i < ui_c.state.rows.size(); i++) {
    auto& row = ui_c.state.rows[i];

    // center the button text
    auto but_size = button_size;
    float pad_x = padding_x;
    bool text_centered = false;
    // if (to_lower(row.col_name).find("back") != std::string::npos)
    // yes to centering for these two buttons
    {
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

    if (selectable_button(r, a_def))
      row.action();
  }

  ImGui::End();
  ImGui::PopFont();
}

} // namespace game2d