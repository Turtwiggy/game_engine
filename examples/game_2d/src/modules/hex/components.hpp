#pragma once

// components
#include "modules/renderer/components.hpp"

// helpers
#include "modules/renderer/helpers/renderers/batch_triangle.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// c++ lib headers
#include <array>
#include <vector>

namespace game2d {

// hex grid consts
constexpr float outer_radius = 24.0f;
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
  int x = 0;
  int y = 0;
  int z = 0;
};

struct HexMesh
{
  std::vector<engine::triangle_renderer::TriangleDescriptor> triangles;
};

struct HexCell
{
  HexPos pos;
  HexCoord coord;
  HexMesh mesh;
};

} // namespace game2d