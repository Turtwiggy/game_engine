// your header
#include "systems/hex_grid_system.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hex_cell.hpp"

// helpers
#include "helpers/hex_grid.hpp"
#include "helpers/renderers/batch_triangle.hpp"

// other project headers
#include <glm/glm.hpp>

// c++ lib headers
#include <iostream>
#include <vector>

// based on:
// https://catlikecoding.com/unity/tutorials/hex-map/part-1/

namespace game2d {

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
  cell.colour = colour;

  entt::entity r = registry.create();
  registry.emplace<HexCell>(r, cell);
  // registry.emplace<HexPos>(r, pos);
  // registry.emplace<HexCoord>(r, coord);
  // registry.emplace<HexMesh>(r, mesh);
  // registry.emplace<Colour>(r, colour);

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
