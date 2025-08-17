#include "pch.hpp"

#include "resources/data.hpp"
#include "ui_land_on_island_popup_helpers.hpp"

namespace game2d {

void
draw_popup(entt::registry& r, const ImVec2 tl, std::string text, ImFont* font, ImU32 border_col)
{
  auto* draw_list = ImGui::GetWindowDrawList();

  // add a background
  const float padding = 4;
  const auto size = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, text.c_str());
  const auto ss_pos_br = tl + size;

  const auto popup_tl = ImVec2{ tl.x - padding, tl.y - padding };
  const auto popup_br = ImVec2{ ss_pos_br.x + padding, ss_pos_br.y + padding };
  draw_list->AddRectFilled(popup_tl, popup_br, im_window_bg_col, 0.0f);
  draw_list->AddRect(popup_tl, popup_br, border_col, 0.0f, ImDrawFlags_RoundCornersNone, 1.0f);

  // add text
  ImGui::SetCursorScreenPos({ tl.x, tl.y });
  ImGui::Text("%s", text.c_str());
}

} // namespace game2d