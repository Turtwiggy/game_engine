#pragma once

#include "engine/colour/colour.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

constexpr int n_players = 4;

const engine::SRGBColour default_player_colours[n_players] = {
  hex_to_srgb("#E8AE58"), // orangeish
  hex_to_srgb("#42B14F"), // green
  hex_to_srgb("#D075CE"), // pink
  hex_to_srgb("#00A8EB"), // blue
};

const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const auto my_inactive_col = hex_to_srgb("#737a7e");
const auto my_separator_col = hex_to_srgb("#7d8488");
const auto my_window_bg_col = hex_to_srgb("#0c1116");
const auto my_window_border_col = hex_to_srgb("#3B5676");

const auto im_inactive_col = convert_my_to_im_vec(my_inactive_col);
const auto im_separator_col = convert_my_to_im_vec(my_separator_col);
const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
const auto im_window_border_col = convert_my_to_im(my_window_bg_col);
const auto im_gold_col = ImVec4(255 * 1.0f, 238 * 1.0f, 0 * 1.0f, 1.0f);
const auto im_text_col = ImVec4(255 * 1.0f, 255 * 1.0f, 255 * 1.0f, 1.0f);
const auto im_icon_col =
  ImVec4(my_separator_col.r / 255.0f, my_separator_col.g / 255.0f, my_separator_col.b / 255.0f, (80 / 255.0f));
const ImVec4 aquired_col = convert_my_to_im_vec(engine::SRGBColour(0, 255, 0, 255));
const ImVec4 unaquired_col = convert_my_to_im_vec(my_inactive_col);

} // namespace game2d