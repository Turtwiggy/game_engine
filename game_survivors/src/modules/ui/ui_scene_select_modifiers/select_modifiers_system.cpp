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
#include "resources/data.hpp"

namespace game2d {

void
back(entt::registry& r)
{
  move_to_scene_start(r, Scene::menu);
};

void
update_ui_scene_select_modifiers_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;
  const auto& ui_scale = get_first_component<SINGLE_UIScaling>(r);
  auto& ui_c = gesert_component<SINGLE_UISelectModifiersMenuState>(r);

  if (!ui_c.init)
    ui_c.do_init(r);

  ui_c.update<RequestToShowModifierMenu>(r);
  if (!ui_c.open)
    return;

  process_input_for_ui_all_handles(r, ui_c.state);
  const auto& acts = ui_c.state.actions;
  const bool do_act = std::find(acts.begin(), acts.end(), UIAction::SELECT) != acts.end();
  const bool do_back = std::find(acts.begin(), acts.end(), UIAction::BACK) != acts.end();
  const bool h_value_changed_l = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_L) != acts.end();
  const bool h_value_changed_r = std::find(acts.begin(), acts.end(), UIAction::NAV_MOVE_R) != acts.end();
  const bool h_value_changed = h_value_changed_l || h_value_changed_r;

  if (do_back) {
    back(r);
    return;
  }

  const auto font_size = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);
  ImGui::PushFont(font, font_size);

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
  const auto rounding = 12.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(window_tl, window_br, im_window_bg_col, rounding);

  const auto TEXT_SIZE = font->CalcTextSizeA(font_size, FLT_MAX, -1, "A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };
  const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
  const float padding_x = 10;

  const auto h_txt = "- GENERAL -";
  const auto h_pos = center_text(font, font_size, h_txt, { window_wh.x * 0.5f, 0 });
  ImGui::SetCursorPosX(h_pos.x);
  ImGui::TextColored(white_col, h_txt);

  for (int i = 0; i < (int)(ui_c.state.cells.size()); i++) {
    auto& cell = ui_c.state.cells[i];
    const bool is_next_button = ui_c.state.cells[i]->name.find("Next") != std::string::npos;

    auto a_def = SelectableButtonDef{
      .display_str = to_upper(cell->name),
      .imgui_hash = "##" + to_upper(cell->name),
      .size = button_size,
      .input = do_act,
      .cell = cell,
      .active_cell = ui_c.state.active,

      .text_pivot = { 0.0f, 0.0f },
      .text_offset = { padding_x, 0 },
      .font = font,
      .font_size = font_size,

      // hide the buttons
      .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    if (selectable_button(r, a_def)) {
      cell->action();

      if (is_next_button)
        break;
    }

    if (is_next_button)
      continue; // dont update the next button

    const auto mod = magic_enum::enum_cast<MODIFIER_OPTIONS>(i).value();
    auto option = get_modifier_option(r, mod);
    if (option == nullptr)
      continue; // option not impl?

    // Update option...
    const bool active = cell == ui_c.state.active;
    if (active && h_value_changed) {
      auto& h_value = dynamic_cast<OptionsCell*>(cell.get())->value;

      if (h_value_changed_r)
        h_value++;
      if (h_value_changed_l)
        h_value--;

      option->update(r, h_value);
    }

    const auto display_button = [&r, window_wh](auto& o, bool& data, const auto label) {
      ImGui::SameLine(window_wh.x * 0.75f);
      if (ImGui::Checkbox(label, &data)) {
        int tmp = (int)data;
        o->update(r, tmp);
      }
    };

    const auto display_increment = [&r, window_wh](auto& o, int& h_value, const float& data, const std::string label) {
      ImGui::SameLine(window_wh.x * 0.75f);

      float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
      if (ImGui::ArrowButton(std::string{ label + "_l" }.c_str(), ImGuiDir_Left)) {
        h_value--;
        o->update(r, h_value);
      }

      ImGui::SameLine(0.0f, spacing);
      // const auto display_txt = o->display_val();
      ImGui::Text("%0.2fx", data);

      ImGui::SameLine(0.0f, spacing);
      if (ImGui::ArrowButton(std::string{ label + "_r" }.c_str(), ImGuiDir_Right)) {
        h_value++;
        o->update(r, h_value);
      }
    };

    // todo: base it of the option type
    // if (option->value_type == UIValueType::BUTTON)

    if (auto* o = dynamic_cast<Option_Rocks*>(option.get()))
      display_button(o, o->populate_rocks, "##rocks");

    if (auto* o = dynamic_cast<Option_EnemyHealth*>(option.get())) {
      auto& h_value = dynamic_cast<OptionsCell*>(cell.get())->value;

      float data = o->multiplier;
      display_increment(o, h_value, data, "##enemy_health");
      o->multiplier = data;
    }

    if (auto* o = dynamic_cast<Option_EnemyCount*>(option.get())) {
      auto& h_value = dynamic_cast<OptionsCell*>(cell.get())->value;
      float data = o->multiplier;
      display_increment(o, h_value, data, "##enemy_count");
      o->multiplier = data;
    }
  }

  ImGui::End();
  ImGui::PopFont();
  ImGui::PopStyleVar(2);
}

} // namespace game2d