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
#include "helpers/hex_grid.hpp"
#include "helpers/renderers/batch_triangle.hpp"
#include "helpers/spritemap.hpp"

// other project headers
#include "imgui.h"
#include <glm/glm.hpp>

// c++ lib headers
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
    registry.emplace<PositionInt>(r, int(x + text_x_offset), int(y));
    registry.emplace<Size>(r, 10, 10);
    registry.emplace<Sprite>(r, number);
    registry.emplace<ZIndex>(r, 1);
    registry.emplace<HexText>(r);

    text_x_offset += text_seperation;
  }
};

void
create_hex_cell(entt::registry& registry, const int x, const int y, const int i, glm::ivec2 offset)
{
  Resources& res = registry.ctx<Resources>();

  // hex position in worldspace
  HexPos pos;
  pos.x = offset.x + (x + y * 0.5f - y / 2) * inner_radius * 2.0f;
  pos.y = offset.y + y * outer_radius * 1.5f;

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

  // // make grid curved
  // if (glm::abs(coord.x) > half_width || glm::abs(coord.y) > half_width || glm::abs(coord.z) > half_width)
  //   return;

  HexCell cell;
  cell.pos = pos;
  cell.coord = coord;
  cell.mesh = mesh;

  HexData data;

  entt::entity r = registry.create();
  registry.emplace<HexCell>(r, cell);
  registry.emplace<HexData>(r, data);

  // render numbers on top of hexagon
  // render_number(registry, coord.x, pos.x, pos.y - inner_radius / 2);
  // render_number(registry, coord.y, pos.x, pos.y);
  // render_number(registry, coord.z, pos.x, pos.y + inner_radius / 2);
};

}; // namespace game2d

void
game2d::init_hex_grid_system(entt::registry& registry, const glm::ivec2& screen_wh)
{
  // destroys all hexagons
  {
    auto view = registry.view<HexCell>();
    registry.destroy(view.begin(), view.end());
  }
  // destroys all text
  {
    auto view = registry.view<HexText>();
    registry.destroy(view.begin(), view.end());
  }

  glm::ivec2 offset = { screen_wh.x / 2, screen_wh.y / 2 };
  int half_width = 20;
  int half_height = 12;

  int i = 0;
  for (int y = -half_height; y <= half_height; y++) {
    for (int x = -half_width; x <= half_width; x++) {
      create_hex_cell(registry, x, y, i++, offset);
    }
  }
};

void
game2d::update_hex_grid_system(entt::registry& registry, engine::Application& app, float dt)
{
  glm::ivec2 screen_wh = app.get_window().get_size();

  static float col[4] = { 0.4f, 0.7f, 0.0f, 0.5f };
  ImGui::Begin("Hex colour picker");
  {
    ImGui::ColorEdit4("My Color", (float*)&col);
    if (ImGui::Button("Reset")) {
      init_hex_grid_system(registry, screen_wh);
    }
  }
  ImGui::End();

  if (app.get_input().get_mouse_lmb_held()) {

    const auto mouse_pos = app.get_input().get_mouse_pos();
    glm::ivec2 offset = { screen_wh.x / 2, screen_wh.y / 2 };
    glm::ivec3 hex_pos = convert_world_pos_to_hex_pos(mouse_pos, offset);
    // std::cout << "(grid pos) " << ix << " " << iy << " " << iz << std::endl;

    // Get the hexmesh, and update it's triangles colours
    Resources& res = registry.ctx<Resources>();
    float rnd = engine::rand_det_s(res.rnd.rng, 0.6f, 0.7f);
    glm::vec4 glm_col = glm::vec4(col[0], col[1], col[2], col[3]);
    glm_col *= rnd;
    glm_col.a = 1.0f;

    const auto view = registry.view<HexCell>();
    view.each([&registry, &hex_pos, &glm_col](auto& cell) {
      if (cell.coord.x == hex_pos.x && cell.coord.y == hex_pos.y && cell.coord.z == hex_pos.z) {
        // Change triangle colour
        for (auto& tri : cell.mesh.triangles) {
          tri.point_0_colour = tri.point_1_colour = tri.point_2_colour = glm_col;
        }
      }
    });
  }
};
