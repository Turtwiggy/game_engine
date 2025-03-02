#pragma once

#include "engine/colour/colour.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

namespace game2d {

engine::SRGBColour
hex_to_srgb(const std::string& hex);

ImVec4
convert_my_to_im_vec(const engine::SRGBColour& col);

ImU32
convert_my_to_im(const engine::SRGBColour& col);

} // namespace game2d