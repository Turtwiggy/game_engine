// your header
#include "select_objects_highlight.hpp"

#include "engine/colour.hpp"

// components
#include "game/components/selectable.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"

void
game2d::update_select_objects_highlight_system(entt::registry& registry)
{
  const auto& view = registry.view<SelectableComponent, HighlightComponent, SpriteComponent>();
  view.each([](auto entity, const auto& s, const auto& highlight, auto& sprite) {
    if (s.is_selected)
      sprite.colour = engine::SRGBToLinear(highlight.highlight_colour);
    else
      sprite.colour = engine::SRGBToLinear(highlight.start_colour);
  });
};
