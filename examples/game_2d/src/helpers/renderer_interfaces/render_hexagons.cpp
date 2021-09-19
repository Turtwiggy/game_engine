// your header
#include "helpers/renderer_interfaces/render_hexagons.hpp"

// components
#include "components/hex_cell.hpp"

// c++ lib headers
#include <vector>

namespace game2d {

std::vector<engine::triangle_renderer::TriangleDescriptor>
RenderHexagons::get_triangles(entt::registry& registry)
{
  std::vector<engine::triangle_renderer::TriangleDescriptor> tris;

  const auto view = registry.view<const HexMesh>();

  view.each([&tris](const auto& mesh) {
    for (const auto& triangle : mesh.triangles) {
      tris.push_back(triangle);
    }
  });

  return tris;
};

} // namespace game2d
