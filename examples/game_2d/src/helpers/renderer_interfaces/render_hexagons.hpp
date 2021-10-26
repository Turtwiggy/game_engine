#pragma once

// game2d headers
#include "helpers/renderers/batch_triangle.hpp"

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

struct RenderHexagons : public engine::triangle_renderer::ITriangleRenderable
{
  std::vector<engine::triangle_renderer::TriangleDescriptor> get_triangles(entt::registry& registry);
};

} // namespace game2d