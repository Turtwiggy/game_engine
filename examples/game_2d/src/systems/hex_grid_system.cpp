// your header
#include "systems/hex_grid_system.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hex_cell.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"
#include "components/z_index.hpp"

// helpers
#include "helpers/renderers/batch_triangle.hpp"
#include "helpers/spritemap.hpp"

// other project headers
#include <glm/glm.hpp>

// c++ lib headers
#include <iostream>
#include <vector>

namespace game2d {

void
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
      registry.emplace<PositionInt>(r, int(pos_x) + x_sprite_x_offset, int(pos_y) - int(inner_radius / 2));
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
      registry.emplace<PositionInt>(r, int(pos_x) + y_sprite_x_offset, int(pos_y));
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
      registry.emplace<PositionInt>(r, int(pos_x) + z_sprite_x_offset, int(pos_y) + int(inner_radius / 2));
      registry.emplace<Size>(r, 10, 10);
      registry.emplace<Sprite>(r, number);
      registry.emplace<ZIndex>(r, 1);

      z_sprite_x_offset += text_seperation;
    }
  }
}

void
create_hex_cell(entt::registry& registry, const int x, const int y, const int i)
{
  HexPos pos;
  HexCoord coord;
  HexMesh mesh;
  Colour colour = { 1.0f, 1.0f, 1.0f, 1.0f };

  // hex position in worldspace
  float offset = 300.0f;
  pos.x = offset + (x + y * 0.5f - y / 2) * inner_radius * 2.0f;
  pos.y = offset + y * outer_radius * 1.5f;

  // hex coords in hexspace
  coord.x = x - (y / 2);
  coord.y = y;
  coord.z = -coord.x - coord.y;

  // hex mesh with create triangles that make up hex
  for (int i = 0; i < 6; i++) {
    const glm::vec2 center = { pos.x, pos.y };
    engine::triangle_renderer::TriangleDescriptor t;
    t.point_0 = center;
    t.point_1 = center + hexagon_pointy_side_up[i];
    t.point_2 = center + hexagon_pointy_side_up[i + 1];
    t.point_0_colour = colour.colour;
    t.point_1_colour = colour.colour;
    t.point_2_colour = colour.colour;

    mesh.triangles[i] = t;
  }

  const int sum_coords = coord.x + coord.y + coord.z;
  if (sum_coords != 0)
    std::cerr << "Error: HexGrid create_hex_cell() cells do not add up to 0" << std::endl;

  // make grid curved
  if (glm::abs(coord.x) > half_width || glm::abs(coord.y) > half_width || glm::abs(coord.z) > half_width)
    return;

  HexCell cell;
  cell.pos = pos;
  cell.coord = coord;
  cell.mesh = mesh;
  // cell.colour = colour;

  entt::entity r = registry.create();
  registry.emplace<HexMesh>(r, mesh);
  // registry.emplace<HexPos>(r, pos);

  render_numbers_on_hexagon(registry, cell);
};

}; // namespace game2d

void
game2d::init_hex_grid_system(entt::registry& registry)
{
  int i = 0;
  for (int y = -half_height; y <= half_height; y++) {
    for (int x = -half_width; x <= half_width; x++) {
      create_hex_cell(registry, x, y, i++);
    }
  }
};

void
game2d::update_hex_grid_system(entt::registry& registry, engine::Application& app, float dt){
  //
};
