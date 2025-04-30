#pragma once

#include "imgui.h"
#include <entt/fwd.hpp>
#include <unordered_map>

namespace game2d {

// From 720p => 1080p, multiply ui by some scale. it could be 1.5
constexpr float scale_size = 1.25f;

enum class FontSize
{
  TEXT_SMALL = 12,
  TEXT_SMALL_SCALED = static_cast<int>(TEXT_SMALL * scale_size),

  TEXT_SIZE_13 = 13,
  TEXT_SIZE_13_SCALED = static_cast<int>(TEXT_SIZE_13 * scale_size),

  // below 16 starts to become too small to read

  TEXT_SIZE_16 = 16,
  TEXT_SIZE_16_SCALED = static_cast<int>(TEXT_SIZE_16 * scale_size),

  TEXT_SIZE_20 = 20,
  TEXT_SIZE_20_SCALED = static_cast<int>(TEXT_SIZE_20 * scale_size),

  TEXT_LARGE = 32,
  TEXT_LARGE_SCALED = static_cast<int>(TEXT_LARGE * scale_size),

  HEADER = 100,
  HEADER_SCALED = static_cast<int>(HEADER * scale_size),
  // note: >150 seems to not load or something.

  MENU_BUTTONS = 40,
  MENU_BUTTONS_SCALED = static_cast<int>(MENU_BUTTONS * scale_size),
};

static std::unordered_map<FontSize, ImFont*> loaded_fonts;

ImFont*
get_inter_font(entt::registry& r, const FontSize size);

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