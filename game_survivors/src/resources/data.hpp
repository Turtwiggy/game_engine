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
const ImU32 default_player_colours_im[n_players] = {
  convert_my_to_im(default_player_colours[0]),
  convert_my_to_im(default_player_colours[1]),
  convert_my_to_im(default_player_colours[2]),
  convert_my_to_im(default_player_colours[3]),
};
const ImU32 default_player_colours_im_transparent[n_players] = {
  convert_my_to_im({ default_player_colours[0].r, default_player_colours[0].g, default_player_colours[0].b, 100 }),
  convert_my_to_im({ default_player_colours[1].r, default_player_colours[1].g, default_player_colours[1].b, 100 }),
  convert_my_to_im({ default_player_colours[2].r, default_player_colours[2].g, default_player_colours[2].b, 100 }),
  convert_my_to_im({ default_player_colours[3].r, default_player_colours[3].g, default_player_colours[3].b, 100 }),
};

const ImU32 im_white = IM_COL32(255, 255, 255, 255);
const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const auto my_inactive_col = hex_to_srgb("#737a7e");
const auto my_separator_col = hex_to_srgb("#7d8488");
const auto my_window_bg_col = engine::SRGBColour(0.13f, 0.14f, 0.17f, 1.00f);
const auto my_window_border_col = hex_to_srgb("#3B5676");

const auto im_text_col_vec = ImVec4{ 0.86f, 0.93f, 0.89f, 0.78f };
const auto im_text_col =
  IM_COL32(im_text_col_vec.x * 255, im_text_col_vec.y * 255, im_text_col_vec.z * 255, im_text_col_vec.z * 255);
const auto im_text_disabled_col = ImVec4{ 0.86f, 0.93f, 0.89f, 0.28f };

const auto im_inactive_col = convert_my_to_im_vec(my_inactive_col);
const auto im_separator_col = convert_my_to_im_vec(my_separator_col);
const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
const auto im_window_border_col = convert_my_to_im(my_window_border_col);
const auto im_gold_col = ImVec4(255 * 1.0f, 238 * 1.0f, 0 * 1.0f, 1.0f);
const auto im_icon_col =
  ImVec4(my_separator_col.r / 255.0f, my_separator_col.g / 255.0f, my_separator_col.b / 255.0f, (80 / 255.0f));

const ImU32 aquired_col = convert_my_to_im(engine::SRGBColour(0, 255, 0, 255));
const ImVec4 aquired_col_vec = convert_my_to_im_vec(engine::SRGBColour(0, 255, 0, 255));
const ImU32 unaquired_col = convert_my_to_im(my_inactive_col);
const ImVec4 unaquired_col_vec = convert_my_to_im_vec(my_inactive_col);

const auto my_text_col_inactive = engine::SRGBColour(200, 200, 200, 100);
const auto im_text_col_inactive = convert_my_to_im(my_text_col_inactive);
const auto my_greenish = hex_to_srgb("#71BBB2");
const auto im_greenish = convert_my_to_im(my_greenish);
const auto im_greenish_vec = convert_my_to_im_vec(my_greenish);

const auto my_w_col = hex_to_srgb("#46C74F");
const auto my_l_col = hex_to_srgb("#DF9755");
const auto im_w_col = convert_my_to_im_vec(my_w_col);
const auto im_l_col = convert_my_to_im_vec(my_l_col);

const auto my_hp_col = hex_to_srgb("#E8AE58", 200);
const auto im_hp_col = convert_my_to_im(my_hp_col);
const auto im_hp_col_vec = convert_my_to_im_vec(my_hp_col);

} // namespace game2d