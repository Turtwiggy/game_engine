#pragma once

// components
#include "components/colour.hpp"
#include "components/hex_cell.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"
#include "components/z_index.hpp"

// game2d headers
#include "helpers/spritemap.hpp"

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

// add sprites for numbers to display on hex
inline void
render_numbers_on_hexagon(entt::registry& registry, const HexCell& hex_cell)
{
  Colour text_colour;
  text_colour.colour = { 0.0f, 0.0f, 0.0f, 1.0f };
  Colour neg_text_colour;
  neg_text_colour.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

  std::vector<sprite::type> x_sprites = convert_int_to_sprites(glm::abs(hex_cell.coord.x));
  bool x_is_negative = hex_cell.coord.x < 0;
  std::vector<sprite::type> y_sprites = convert_int_to_sprites(glm::abs(hex_cell.coord.y));
  bool y_is_negative = hex_cell.coord.y < 0;
  std::vector<sprite::type> z_sprites = convert_int_to_sprites(glm::abs(hex_cell.coord.z));
  bool z_is_negative = hex_cell.coord.z < 0;

  int x_sprite_x_offset = 0;
  int y_sprite_x_offset = 0;
  int z_sprite_x_offset = 0;
  const int text_seperation = 6;

  float pos_x = hex_cell.pos.x;
  float pos_y = hex_cell.pos.y;

  { // x pos
    for (auto& number : x_sprites) {
      entt::entity r = registry.create();
      if (x_is_negative)
        registry.emplace<Colour>(r, neg_text_colour);
      else
        registry.emplace<Colour>(r, text_colour);
      registry.emplace<PositionInt>(r, pos_x + x_sprite_x_offset, pos_y - inner_radius / 2);
      registry.emplace<Size>(r, 10, 10);
      registry.emplace<Sprite>(r, number);
      registry.emplace<ZIndex>(r, 1);

      x_sprite_x_offset += text_seperation;
    }
  }
  { // y pos
    for (auto& number : y_sprites) {
      entt::entity r = registry.create();
      if (y_is_negative)
        registry.emplace<Colour>(r, neg_text_colour);
      else
        registry.emplace<Colour>(r, text_colour);
      registry.emplace<PositionInt>(r, pos_x + y_sprite_x_offset, pos_y);
      registry.emplace<Size>(r, 10, 10);
      registry.emplace<Sprite>(r, number);
      registry.emplace<ZIndex>(r, 1);

      y_sprite_x_offset += text_seperation;
    }
  }
  { // z pos
    for (auto& number : z_sprites) {
      entt::entity r = registry.create();
      if (z_is_negative)
        registry.emplace<Colour>(r, neg_text_colour);
      else
        registry.emplace<Colour>(r, text_colour);
      registry.emplace<PositionInt>(r, pos_x + z_sprite_x_offset, pos_y + inner_radius / 2);
      registry.emplace<Size>(r, 10, 10);
      registry.emplace<Sprite>(r, number);
      registry.emplace<ZIndex>(r, 1);

      z_sprite_x_offset += text_seperation;
    }
  }
}

} // game2d