// your header
#include "systems/hover_system.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hoverable.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"

// other lib headers
#include <glm/glm.hpp>

void
game2d::update_hover_system(entt::registry& registry, engine::Application& app)
{
  auto& input = app.get_input();
  auto& mouse_pos = input.get_mouse_pos();

  auto& view = registry.view<Hoverable, Position, Size, Colour>();
  view.each(
    [&mouse_pos](auto entity, Hoverable& hoverable, const Position& position, const Size& size, Colour& colour) {
      if (mouse_pos.x >= position.x - int(size.w / 2.0f) && mouse_pos.x <= position.x + int(size.w / 2.0f) &&
          mouse_pos.y <= position.y + int(size.h / 2.0f) && mouse_pos.y >= position.y - int(size.h / 2.0f)) {
        if (!hoverable.hovering) {
          hoverable.hovering = true;
          hoverable.normal_colour.colour = colour.colour;
        }
        colour.colour = hoverable.hover_colour.colour;
      } else {
        hoverable.hovering = false;
        colour.colour = hoverable.normal_colour.colour;
      }
    });
}