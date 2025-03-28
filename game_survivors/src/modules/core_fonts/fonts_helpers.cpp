#include "pch.hpp"

#include "fonts_helpers.hpp"

namespace game2d {

// https://github.com/ocornut/imgui/issues/8465

ImFont*
get_fingerpaint_font(entt::registry& r, const FontSize size)
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

  const int count = magic_enum::enum_count<FontSize>();
  for (int i = 0; i < count; i++) {
    const FontSize size_enum = magic_enum::enum_value<FontSize>(i);
    const auto size = (int)size_enum;

    auto* font = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter-VariableFont.ttf", (float)size);
    loaded_fonts[size_enum] = font;
  }
}

} // namespace game2d