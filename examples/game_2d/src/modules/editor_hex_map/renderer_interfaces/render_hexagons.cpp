// your header
#include "render_hexagons.hpp"

// components
#include "modules/editor_hex_map/components.hpp"

// c++ lib headers
#include <vector>

namespace game2d {

std::vector<engine::triangle_renderer::TriangleDescriptor>
RenderHexagons::get_triangles(entt::registry& registry)
{
  std::vector<engine::triangle_renderer::TriangleDescriptor> tris;

  const auto& view = registry.view<const HexCell>();

  view.each(
    [&tris](const auto& cell) { tris.insert(tris.end(), cell.mesh.triangles.begin(), cell.mesh.triangles.end()); });

  return tris;
};

} // namespace game2d
