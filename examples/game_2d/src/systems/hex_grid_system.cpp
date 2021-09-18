// your header
#include "systems/hex_grid_system.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hex_cell.hpp"
#include "components/hoverable.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"
#include "components/z_index.hpp"

// helpers
#include "helpers/renderers/batch_triangle.hpp"

// engien project headers
#include "engine/maths_core.hpp"

// other project headers
#include <GL/glew.h> // temp while messing with opengl
#include <glm/glm.hpp>

// c++ lib headers
#include <iostream>
#include <vector>

// based on:
// https://catlikecoding.com/unity/tutorials/hex-map/part-1/

namespace game2d {

constexpr float outer_radius = 32.0f;
constexpr float inner_radius = outer_radius * 0.866025404f;
constexpr int half_width = 5;
constexpr int half_height = 5;

const std::array<glm::vec2, 7> hexagon_pointy_side_up = {
  glm::vec2(0.0f, outer_radius),
  glm::vec2(inner_radius, 0.5f * outer_radius),
  glm::vec2(inner_radius, -0.5f * outer_radius),
  glm::vec2(0.0f, -outer_radius),
  glm::vec2(-inner_radius, -0.5f * outer_radius),
  glm::vec2(-inner_radius, 0.5f * outer_radius),
  glm::vec2(0.0f, outer_radius), // last point is first point
};

void
create_hex_cell(entt::registry& registry, const glm::vec2 offset, const int x, const int y, const int i)
{
  // hex position in worldspace
  const float pos_x = offset.x + (x + y * 0.5f - y / 2) * inner_radius * 2.0f;
  const float pos_y = offset.y + y * outer_radius * 1.5f;

  // hex coords in hexspace
  const int x_coord = x - (y / 2);
  const int y_coord = y;
  const int z_coord = -x_coord - y_coord;

  const int sum_coords = x_coord + y_coord + z_coord;
  if (sum_coords != 0)
    std::cerr << "Error: HexGrid create_hex_cell() cells do not add up to 0" << std::endl;

  // make grid curved
  if (glm::abs(x_coord) > half_width || glm::abs(y_coord) > half_width || glm::abs(z_coord) > half_width)
    return;

  entt::entity r = registry.create();
  registry.emplace<PositionFloat>(r, pos_x, pos_y);
  registry.emplace<HexCell>(r, i, x_coord, y_coord, z_coord);
  registry.emplace<Size>(r, outer_radius, outer_radius);
};

// add sprites for numbers to display on hex
void
render_numbers_on_hexagon(entt::registry& registry, const HexCell& hex_cell, const PositionFloat& pos)
{
  Colour text_colour;
  text_colour.colour = { 0.0f, 0.0f, 0.0f, 1.0f };
  Colour neg_text_colour;
  neg_text_colour.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

  std::vector<sprite::type> x_sprites = convert_int_to_sprites(glm::abs(hex_cell.coord_x));
  bool x_is_negative = hex_cell.coord_x < 0;
  std::vector<sprite::type> y_sprites = convert_int_to_sprites(glm::abs(hex_cell.coord_y));
  bool y_is_negative = hex_cell.coord_y < 0;
  std::vector<sprite::type> z_sprites = convert_int_to_sprites(glm::abs(hex_cell.coord_z));
  bool z_is_negative = hex_cell.coord_z < 0;

  int x_sprite_x_offset = 0;
  int y_sprite_x_offset = 0;
  int z_sprite_x_offset = 0;
  const int text_seperation = 6;

  { // x pos
    for (auto& number : x_sprites) {
      entt::entity r = registry.create();
      if (x_is_negative)
        registry.emplace<Colour>(r, neg_text_colour);
      else
        registry.emplace<Colour>(r, text_colour);
      registry.emplace<PositionInt>(r, pos.x + x_sprite_x_offset, pos.y - inner_radius / 2);
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
      registry.emplace<PositionInt>(r, pos.x + y_sprite_x_offset, pos.y);
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
      registry.emplace<PositionInt>(r, pos.x + z_sprite_x_offset, pos.y + inner_radius / 2);
      registry.emplace<Size>(r, 10, 10);
      registry.emplace<Sprite>(r, number);
      registry.emplace<ZIndex>(r, 1);

      z_sprite_x_offset += text_seperation;
    }
  }
}

void
triangulate(entt::registry& registry)
{
  Resources& res = registry.ctx<Resources>();

  // destroy all triangles - could be pretty risky?
  {
    auto view = registry.view<engine::triangle_renderer::TriangleDescriptor>();
    registry.destroy(view.begin(), view.end());
  }

  auto view = registry.view<const HexCell, const PositionFloat>();
  view.each([&res, &registry](const auto& hex_cell, const auto& pos) {
    //
    // generate random colour
    // float rnd = engine::rand_det_s(res.rnd.rng, 0.6f, 0.7f);
    float rnd = 1.0f;
    //
    // create triangles that make up hex
    for (int i = 0; i < 6; i++) {
      entt::entity entity = registry.create();

      const glm::vec2 center = { pos.x, pos.y };
      engine::triangle_renderer::TriangleDescriptor t;
      t.point_0 = center;
      t.point_1 = center + hexagon_pointy_side_up[i];
      t.point_2 = center + hexagon_pointy_side_up[i + 1];
      t.point_0_colour = t.point_1_colour = t.point_2_colour = glm::vec4(rnd, rnd, rnd, 1.0f);

      registry.emplace<engine::triangle_renderer::TriangleDescriptor>(entity, t);
    }

    render_numbers_on_hexagon(registry, hex_cell, pos);
  });
}

} // namespace game2d

void
game2d::init_hex_grid_system(entt::registry& registry, glm::vec2 screen_wh)
{
  glm::vec2 offset = screen_wh / 2.0f;

  int i = 0;
  for (int y = -half_height; y <= half_height; y++) {
    for (int x = -half_width; x <= half_width; x++) {
      create_hex_cell(registry, offset, x, y, i++);
    }
  }

  triangulate(registry);
};

void
game2d::update_hex_grid_system(entt::registry& registry, float dt){
  // const float wait_seconds = 1;
  // float wait_seconds_remaining = 1;
  // bool do_thing = false;
  // wait_seconds_remaining -= dt;
  // if (wait_seconds_remaining <= 0.0f) {
  //   wait_seconds_remaining = wait_seconds;
  //   do_thing = true;
  // }

  // if (do_thing) {
  //   do_thing = false;
  //   triangulate(registry);
  // }

  // ImGui::Begin("Triangulate");
  // if (ImGui::Button("Do Triangulate")) {
  //   triangulate(registry);
  // }
  // ImGui::End();
};
