#pragma once

#include "imgui.h"
#include <entt/fwd.hpp>
#include <unordered_map>

namespace game2d {

enum class FontSize
{
  TEXT_SMALL = 12,
  TEXT_SMALL_SCALED = 18,

  TEXT_DESCRIPTION = 16,
  TEXT_DESCRIPTION_SCALED = 24,

  TEXT_MEDIUM = 20,        // 720p
  TEXT_MEDIUM_SCALED = 30, // 1080p

  TEXT_LARGE = 32,        // 720p
  TEXT_LARGE_SCALED = 48, // 1080p

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

} // namespace game2d