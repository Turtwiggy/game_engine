#pragma once

// components
#include "components/colour.hpp"
#include "components/position.hpp"

// helpers
#include "helpers/renderers/batch_triangle.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// c++ lib headers
#include <array>
#include <vector>

namespace game2d {

// hex grid consts
constexpr float outer_radius = 20.0f;
constexpr float inner_radius = outer_radius * 0.866025404f;

constexpr std::array<glm::vec2, 7> hexagon_pointy_side_up = {
  glm::vec2(0.0f, outer_radius),
  glm::vec2(inner_radius, 0.5f * outer_radius),
  glm::vec2(inner_radius, -0.5f * outer_radius),
  glm::vec2(0.0f, -outer_radius),
  glm::vec2(-inner_radius, -0.5f * outer_radius),
  glm::vec2(-inner_radius, 0.5f * outer_radius),
  glm::vec2(0.0f, outer_radius), // last point is first point
};

struct HexText
{};

struct HexPos : public PositionFloat
{};

struct HexCoord
{
  int x, y, z, i = 0;
};

struct HexMesh
{
  std::array<engine::triangle_renderer::TriangleDescriptor, 6> triangles;
};

} // namespace game2d