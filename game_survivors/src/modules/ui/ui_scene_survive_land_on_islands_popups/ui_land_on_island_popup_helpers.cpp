#include "pch.hpp"

#include "ui_land_on_island_popup_helpers.hpp"

namespace game2d {

void
draw_popup(entt::registry& r, const ImVec2 tl, std::string text, ImFont* font)
{
  auto* draw_list = ImGui::GetWindowDrawList();

  // add a background
  const float padding = 4;
  const auto size = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, text.c_str());
  const auto ss_pos_br = tl + size;
  draw_list->AddRectFilled(
    { tl.x - padding, tl.y - padding }, { ss_pos_br.x + padding, ss_pos_br.y + padding }, IM_COL32(0, 0, 0, 200), 6);

  // add text
  ImGui::SetCursorScreenPos({ tl.x, tl.y });
  ImGui::Text("%s", text.c_str());
}

} // namespace game2d