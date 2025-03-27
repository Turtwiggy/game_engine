#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "fonts_component.hpp"
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

  const int count = magic_enum::enum_count<FontSize>();
  for (int i = 0; i < count; i++) {
    const FontSize size_enum = magic_enum::enum_value<FontSize>(i);
    const auto size = (int)size_enum;

    auto* font = io.Fonts->AddFontFromFileTTF("assets/fonts/FingerPaint-Regular.ttf", (float)size);
    loaded_fonts[size_enum] = font;
  }
}

} // namespace game2d