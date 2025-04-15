#pragma once

#include "engine/colour/colour.hpp"

#include <entt/fwd.hpp>
#include <imgui.h>

namespace game2d {

engine::SRGBColour
hex_to_srgb(const std::string& hex, int alpha = 255);

ImVec4
convert_my_to_im_vec(const engine::SRGBColour& col);

ImU32
convert_my_to_im(const engine::SRGBColour& col);

} // namespace game2d