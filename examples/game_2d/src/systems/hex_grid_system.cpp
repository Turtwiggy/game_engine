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
create_hex_cell(entt::registry& registry, int x, int y, int z, int i)
{
  Resources& res = registry.ctx<Resources>();
  // std::cout << "creating cell at: " << x << " " << y << " " << z << " " << i << std::endl;

  const float pos_x = (x + y * 0.5f - y / 2) * inner_radius * 2.0f;
  const float pos_y = y * outer_radius * 1.5f;
  // const int pos_z = z;

  entt::entity r = registry.create();
  registry.emplace<PositionFloat>(r, pos_x, pos_y);
  // hex
  registry.emplace<HexCell>(r, i);
  registry.emplace<Size>(r, outer_radius, outer_radius);
};

void
triangulate(entt::registry& registry)
{
  Resources& res = registry.ctx<Resources>();

  // todo: destroy all triangles
  // destroys all the entities in a range
  // auto view = registry.view<a_component, another_component>();
  // registry.destroy(view.begin(), view.end());

  auto view = registry.view<const HexCell, const PositionFloat>();
  view.each([&res, &registry](const auto& hex_cell, const auto& pos) {
    //
    // generate random colour
    float rnd = engine::rand_det_s(res.rnd.rng, 0.3f, 0.6f);

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
  });
}

} // namespace game2d

void
game2d::init_hex_grid_system(entt::registry& registry)
{
  constexpr int width = 28;
  constexpr int height = 18;

  int i = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      create_hex_cell(registry, x, y, 0, i++);
    }
  }

  triangulate(registry);
};

void
game2d::update_hex_grid_system(entt::registry& registry, float dt){
  //
};
