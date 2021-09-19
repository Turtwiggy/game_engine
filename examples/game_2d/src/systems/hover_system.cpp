// your header
#include "systems/hover_system.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hoverable.hpp"
#include "components/position.hpp"
#include "components/size.hpp"

// helpers
#include "helpers/renderers/batch_triangle.hpp"

// other lib headers
#include <glm/glm.hpp>

void
game2d::update_hover_system(entt::registry& registry, engine::Application& app)
{
  auto& mouse_pos = app.get_input().get_mouse_pos();
  {
    auto& view = registry.view<Hoverable, PositionInt, Size>();
    view.each([&mouse_pos](auto entity, Hoverable& hoverable, const PositionInt& position, const Size& size) {
      if (mouse_pos.x >= position.x - int(size.w / 2.0f) && mouse_pos.x <= position.x + int(size.w / 2.0f) &&
          mouse_pos.y <= position.y + int(size.h / 2.0f) && mouse_pos.y >= position.y - int(size.h / 2.0f)) {
        if (!hoverable.mouse_is_hovering) {
          hoverable.mouse_is_hovering = true;
        }
      } else {
        hoverable.mouse_is_hovering = false;
      }
    });
  }

  // {
  //   auto& view = registry.view<Hoverable, engine::triangle_renderer::TriangleDescriptor>();
  // }
}