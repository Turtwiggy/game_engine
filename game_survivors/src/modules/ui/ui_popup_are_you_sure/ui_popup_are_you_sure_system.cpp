#include "pch.hpp"

#include "ui_popup_are_you_sure_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "resources/data.hpp"
#include "ui_popup_are_you_sure_components.hpp"

namespace game2d {

void
update_ui_popup_are_you_sure_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& ui_c = SINGLE_UIAreYouSure::instance;
  ui_c.open = ui_c.action.has_value();

  if (!ui_c.open)
    return;

  if (!ui_c.init) {

    Cell yes_cell;
    yes_cell.name = "Yes";
    ui_c.state.cells.push_back(std::make_shared<Cell>(yes_cell));

    Cell no_cell;
    no_cell.name = "No";
    ui_c.state.cells.push_back(std::make_shared<Cell>(no_cell));

    // create navlinks
    create_as_horizontal_layout(ui_c.state.cells);

    ui_c.state.active = ui_c.state.cells[0];

    ui_c.init = true;
  }

  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b = g_input_c.button_s;
  const bool do_act = std::find(b.begin(), b.end(), ActionStateEnum::DOWN) != b.end();

  auto* font = get_inter_font(r);
  const auto text_size = (float)FontSizes::SIZE_16;
  const ImVec2 button_size = { 160.0f, (text_size * 2.0f) + 2.0f };

  const auto& ri = SINGLE_RendererInfo::instance;
  const auto window_pos = ImVec2{ ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f };
  ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2{ 0.5f, 0.5f });
  const auto window_size = glm::vec2{ 240, 100 };
  ImGui::SetNextWindowSize({ window_size.x, window_size.y }, ImGuiCond_Always);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0, 0 });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));

  ImGui::SetNextWindowFocus();

  imgui_begin("PopupAreYouSure");

  const auto ui_tl = ImGui::GetWindowPos();
  const auto ui_wh = ImGui::GetWindowSize();
  const auto ui_br = ImVec2{ ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };
  auto* draw_list = ImGui::GetWindowDrawList();

  // background
  const auto rounding = 4.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
  draw_list->AddRect(ui_tl, ui_br, IM_COL32(255, 255, 255, 255), rounding, rect_flags, thickness);

  ImGui::PushFont(font, text_size);
  ImGui::PushStyleColor(ImGuiCol_Separator, im_separator_col);

  std::string label = "Are you sure?";
  float alignment = 0.5f;
  float size = ImGui::CalcTextSize(label.c_str()).x;
  float avail = ImGui::GetContentRegionAvail().x;
  float off = (avail - size) * alignment;
  if (off > 0.0f)
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
  ImGui::Text("%s", label.c_str());

  // ImGui::Separator();
  ImGui::PopStyleColor();
  ImGui::PopFont();

  auto yes_def = SelectableButtonDef{
    .display_str = "Yes",
    .imgui_hash = "##yesbutton",
    .size = button_size,
    .input = do_act,
    .cell = ui_c.state.cells[0],
    .active_cell = ui_c.state.active,
    .font = font,
    .font_size = text_size,
  };
  ImGui::SetCursorPosX(0.5f * (window_size.x - yes_def.size.x)); // padding

  if (selectable_button(r, yes_def)) {
    ui_c.action.value()(true);
    ui_c.action = std::nullopt;
  }

  auto no_def = SelectableButtonDef{
    .display_str = "No",
    .imgui_hash = "##nobutton",
    .size = button_size,
    .input = do_act,
    .cell = ui_c.state.cells[1],
    .active_cell = ui_c.state.active,
    .font = font,
    .font_size = text_size,
  };

  ImGui::SetCursorPosX(0.5f * (window_size.x - no_def.size.x)); // padding
  if (selectable_button(r, no_def)) {
    ui_c.action.value()(false);
    ui_c.action = std::nullopt;
  }

  ImGui::End();
  ImGui::PopStyleVar(5);
}

} // namespace game2d