#include "pch.hpp"

#include "fonts_helpers.hpp"

namespace game2d {

// https://github.com/ocornut/imgui/issues/8465

ImFont*
get_fingerpaint_font(entt::registry& r)
{
  auto* font = ImGui::GetIO().Fonts->Fonts[2];
  return font;
};

ImFont*
get_inter_font(entt::registry& r)
{
  auto* font = ImGui::GetIO().Fonts->Fonts[3];
  return font;
};

void
init_fonts_system()
{
  auto& io = ImGui::GetIO();

  // idx: 0
  io.Fonts->AddFontDefault();

  // idx: 1
  io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 32.0f);

  // idx: 2
  io.Fonts->AddFontFromFileTTF("assets/fonts/FingerPaint-Regular.ttf", 16.0f);

  // idx: 3
  io.Fonts->AddFontFromFileTTF("assets/fonts/Inter-VariableFont.ttf", 16.0f);
}

ImVec2
calc_center(const ImVec2 tl, const ImVec2 wh)
{
  return { tl.x + 0.5f * wh.x, tl.y + 0.5f * wh.y };
};

ImVec2
calc_wh(const ImVec2 tl, const ImVec2 br)
{
  return { glm::abs(br.x - tl.x), glm::abs(br.y - tl.y) };
};

ImVec2
center_text(ImFont* font, float font_size, const std::string& text, const ImVec2& pos, const ImVec2 pivot)
{
  const auto size = font->CalcTextSizeA(font_size, FLT_MAX, -1, text.c_str());
  return pos - ImVec2{ size.x * pivot.x, size.y * pivot.y };
};

} // namespace game2d