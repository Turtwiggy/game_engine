#pragma once

#include <entt/fwd.hpp>
#include <imgui.h>

namespace game2d {

// From 720p => 1080p
constexpr float scale_size = 1.25f;

enum class FontSizes
{
  SIZE_12 = 12,
  SIZE_13 = 13,
  SIZE_16 = 16,
  SIZE_20 = 20,
  SIZE_32 = 32,
  HEADER = 100,
};

ImFont*
get_fingerpaint_font(entt::registry& r);

ImFont*
get_inter_font(entt::registry& r);

void
init_fonts_system();

//

ImVec2
calc_center(const ImVec2 tl, const ImVec2 wh);

ImVec2
calc_wh(const ImVec2 tl, const ImVec2 br);

ImVec2
center_text(ImFont* font, float font_size, const std::string& text, const ImVec2& pos, const ImVec2 pivot = { 0.5f, 0.5f });

} // namespace game2d