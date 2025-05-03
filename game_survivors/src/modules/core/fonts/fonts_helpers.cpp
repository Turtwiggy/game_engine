#include "pch.hpp"

#include "fonts_helpers.hpp"

namespace game2d {

// https://github.com/ocornut/imgui/issues/8465

static std::unordered_map<FontSize, ImFont*> loaded_fonts;

ImFont*
get_inter_font(entt::registry& r, const FontSize size)
{
  return loaded_fonts[size];
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
  io.Fonts->AddFontFromFileTTF("assets/fonts/FingerPaint-Regular.ttf", 100.0f);
  // idx: 3 upgrade menu header
  io.Fonts->AddFontFromFileTTF("assets/fonts/FingerPaint-Regular.ttf", 20.0f);
  // idx: 4 upgrade menu header scaled
  io.Fonts->AddFontFromFileTTF("assets/fonts/FingerPaint-Regular.ttf", 20.0f * scale_size);

  const int count = magic_enum::enum_count<FontSize>();
  for (int i = 0; i < count; i++) {
    const FontSize size_enum = magic_enum::enum_value<FontSize>(i);
    const auto size = (int)size_enum;

    auto* font = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter-VariableFont.ttf", (float)size);
    loaded_fonts[size_enum] = font;
  }
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
center_text(const ImFont* font, const std::string& text, const ImVec2& pos, const ImVec2 pivot)
{
  const auto size = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, text.c_str());
  return pos - ImVec2{ size.x * pivot.x, size.y * pivot.y };
};

} // namespace game2d