#pragma once

#include "engine/colour/colour.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

constexpr int n_players = 4;
constexpr float default_map_tilesize = 20;
constexpr float default_map_unit_tilesize = 16;

const std::string discord_link = "https://discord.gg/8RTzsm25pR";

const std::vector<engine::SRGBColour> default_player_colours = {

  hex_to_srgb("#B3BFFF"), //
  hex_to_srgb("#DD7BDF"), //
  hex_to_srgb("#FFBBE1"), //
  hex_to_srgb("#FFF58A"), //

  hex_to_srgb("#00A8EB"), // blue
  hex_to_srgb("#D075CE"), // pink
  hex_to_srgb("#E8AE58"), // orangeish
  hex_to_srgb("#42B14F"), // green

  hex_to_srgb("#00FFFF"), //
  hex_to_srgb("#E83C91"), //
  hex_to_srgb("#FF5F1F"), //
  hex_to_srgb("#00FFA2"), //
};

const std::vector<ImU32> default_player_colours_im = [] {
  std::vector<ImU32> v;
  auto convert = [](const auto& c) { return convert_my_to_im(c); };
  std::transform(default_player_colours.begin(), default_player_colours.end(), std::back_inserter(v), convert);
  return v;
}();
const std::vector<ImU32> default_player_colours_im_transparent = [] {
  std::vector<ImU32> v;
  auto convert = [](const auto& c) { return convert_my_to_im({ c.r, c.g, c.b, 100 }); };
  std::transform(default_player_colours.begin(), default_player_colours.end(), std::back_inserter(v), convert);
  return v;
}();

const auto my_hp_bar_background_col = hex_to_srgb("#15171B");
const auto im_hp_bar_background_col = convert_my_to_im(my_hp_bar_background_col);

const auto my_white = hex_to_srgb("#FFFFFF");
const ImU32 im_white = IM_COL32(255, 255, 255, 255);
const auto white_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
const auto my_inactive_col = hex_to_srgb("#737a7e");
const auto my_separator_col = hex_to_srgb("#7d8488");
const auto my_window_bg_col = engine::SRGBColour(0.13f, 0.14f, 0.17f, 1.00f);
const auto my_window_border_col = hex_to_srgb("#3B5676");
const auto my_gold_col = hex_to_srgb("#ffee00ff");

const auto my_red = hex_to_srgb("#d74200");
const auto my_orange = hex_to_srgb("#e99f10");
const auto my_reload_col = my_orange;
const auto my_death_sprite_col = hex_to_srgb("#a64a2e");
const auto my_grey = hex_to_srgb("#333333", 10);

const auto im_orange_vec = convert_my_to_im_vec(my_orange);

const auto my_text_col_dark = engine::SRGBColour(23, 23, 23, 255);
const auto my_text_col = engine::SRGBColour(0.86f, 0.93f, 0.89f, 0.78f);
const auto im_text_col_vec = convert_my_to_im_vec(my_text_col);
const auto im_text_col =
  IM_COL32(im_text_col_vec.x * 255, im_text_col_vec.y * 255, im_text_col_vec.z * 255, im_text_col_vec.z * 255);
const auto im_text_disabled_col = ImVec4{ 0.86f, 0.93f, 0.89f, 0.28f };

const auto im_inactive_col = convert_my_to_im(my_inactive_col);
const auto im_inactive_col_vec = convert_my_to_im_vec(my_inactive_col);
const auto im_separator_col = convert_my_to_im_vec(my_separator_col);
const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
const auto im_window_border_col = convert_my_to_im(my_window_border_col);
const auto im_gold_col = convert_my_to_im(my_gold_col);
const auto im_gold_col_vec = convert_my_to_im_vec(my_gold_col);
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
const auto im_w_col = convert_my_to_im(my_w_col);
const auto im_l_col = convert_my_to_im(my_l_col);
const auto im_w_col_vec = convert_my_to_im_vec(my_w_col);
const auto im_l_col_vec = convert_my_to_im_vec(my_l_col);

const auto my_hp_col = hex_to_srgb("#FF69B4.", 200);
const auto im_hp_col = convert_my_to_im(my_hp_col);
const auto im_hp_col_vec = convert_my_to_im_vec(my_hp_col);

const auto my_line_col = hex_to_srgb("#a3adb3ff");
const auto im_line_col = convert_my_to_im(my_line_col);
const auto im_line_col_vec = convert_my_to_im_vec(my_line_col);

} // namespace game2d