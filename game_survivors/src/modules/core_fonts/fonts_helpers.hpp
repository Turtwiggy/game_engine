#pragma once

#include "imgui.h"
#include <entt/fwd.hpp>
#include <unordered_map>

namespace game2d {

// From 720p => 1080p, multiply all by 1.5x
enum class FontSize
{
  TEXT_SMALL = 12,
  TEXT_SMALL_SCALED = 18,

  TEXT_SIZE_13 = 13,
  TEXT_SIZE_13_SCALED = 20, // should be 19.5

  TEXT_SIZE_16 = 16,
  TEXT_SIZE_16_SCALED = 24,

  TEXT_MEDIUM = 20,
  TEXT_MEDIUM_SCALED = 30,

  TEXT_LARGE = 32,
  TEXT_LARGE_SCALED = 48,

  HEADER = 100,
  HEADER_SCALED = 120,
  // note: not x1.5. 150 seems to not load or something.

  MENU_BUTTONS = 40,
  MENU_BUTTONS_SCALED = 60,
};

static std::unordered_map<FontSize, ImFont*> loaded_fonts;

ImFont*
get_fingerpaint_font(entt::registry& r, const FontSize size);

void
init_fonts_system();

//

ImVec2
calc_center(const ImVec2 tl, const ImVec2 wh);

ImVec2
calc_wh(const ImVec2 tl, const ImVec2 br);

ImVec2
center_text(const ImFont* font, const std::string& text, const ImVec2& pos, const ImVec2 pivot = { 0.5f, 0.5f });

} // namespace game2d