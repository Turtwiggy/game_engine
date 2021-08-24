#pragma once

// c++ headers
#include <array>

// other lib headers
#include <imgui.h>

// engine hearders
#include "spritemap.hpp"

namespace game2d {

std::array<ImVec2, 2>
convert_sprite_to_uv(sprite::type type);

} // namespace game2d