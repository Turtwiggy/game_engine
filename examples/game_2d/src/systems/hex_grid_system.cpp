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
render_number(entt::registry& registry, int number, float x, float y)
{
  Colour text_colour;
  text_colour.colour = { 0.0f, 0.0f, 0.0f, 1.0f };
  Colour neg_text_colour;
  neg_text_colour.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

  std::vector<sprite::type> sprites = convert_int_to_sprites(glm::abs(number));
  bool is_negative = number < 0;

  const int text_seperation = 6;
  float text_x_offset = 0;

  for (auto& number : sprites) {
    entt::entity r = registry.create();
    if (is_negative)
      registry.emplace<Colour>(r, neg_text_colour);
    else
      registry.emplace<Colour>(r, text_colour);
    registry.emplace<PositionInt>(r, int(x) + text_x_offset, int(y));
    registry.emplace<Size>(r, 10, 10);
    registry.emplace<Sprite>(r, number);
    registry.emplace<ZIndex>(r, 1);

    text_x_offset += text_seperation;
  }
};

void
create_hex_cell(entt::registry& registry, const int x, const int y, const int i)
{
  Resources& res = registry.ctx<Resources>();

  // hex position in worldspace
  HexPos pos;
  float offset = 300.0f;
  pos.x = offset + (x + y * 0.5f - y / 2) * inner_radius * 2.0f;
  pos.y = offset + y * outer_radius * 1.5f;

  // hex coords in hexspace
  HexCoord coord;
  coord.x = x - (y / 2);
  coord.y = y;
  coord.z = -coord.x - coord.y;

  // hex mesh with create triangles that make up hex (with random colours)
  HexMesh mesh;
  float rnd = engine::rand_det_s(res.rnd.rng, 0.6f, 0.7f);
  for (int i = 0; i < 6; i++) {
    const glm::vec2 center = { pos.x, pos.y };
    engine::triangle_renderer::TriangleDescriptor t;
    t.point_0 = center;
    t.point_1 = center + hexagon_pointy_side_up[i];
    t.point_2 = center + hexagon_pointy_side_up[i + 1];
    t.point_0_colour = glm::vec4(rnd, rnd, rnd, 1.0f);
    t.point_1_colour = glm::vec4(rnd, rnd, rnd, 1.0f);
    t.point_2_colour = glm::vec4(rnd, rnd, rnd, 1.0f);

    mesh.triangles[i] = t;
  }

  const int sum_coords = coord.x + coord.y + coord.z;
  if (sum_coords != 0)
    std::cerr << "Error: HexGrid create_hex_cell() cells do not add up to 0" << std::endl;

  // make grid curved
  if (glm::abs(coord.x) > half_width || glm::abs(coord.y) > half_width || glm::abs(coord.z) > half_width)
    return;

  entt::entity r = registry.create();
  registry.emplace<HexPos>(r, pos);
  registry.emplace<HexCoord>(r, coord);
  registry.emplace<HexMesh>(r, mesh);

  // render numbers on top of hexagon
  render_number(registry, coord.x, pos.x, pos.y - inner_radius / 2);
  render_number(registry, coord.y, pos.x, pos.y);
  render_number(registry, coord.z, pos.x, pos.y + inner_radius / 2);
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
