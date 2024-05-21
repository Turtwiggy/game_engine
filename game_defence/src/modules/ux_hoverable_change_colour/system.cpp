#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "sprites/components.hpp"

namespace game2d {

void
update_ux_hoverable_change_colour_system(entt::registry& r)
{
  // Set colour for anything selected

  const auto& selected_view = r.view<SpriteComponent, TagComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, sc, tag] : selected_view.each()) {

    const auto* selected = r.try_get<SelectedComponent>(e);
    if (selected) {
      if (const auto* preferred_colour = r.try_get<HoveredColour>(e))
        sc.colour = engine::SRGBToLinear(preferred_colour->colour);
      else if (const auto* tag = r.try_get<TagComponent>(e))
        sc.colour = get_lin_colour_by_tag(r, tag->tag);
      continue;
    }

    const auto* hovered = r.try_get<HoveredComponent>(e);
    if (!selected && hovered) {
      if (const auto* preferred_colour = r.try_get<HoveredColour>(e))
        sc.colour = engine::SRGBToLinear(preferred_colour->colour);
      else
        sc.colour = get_lin_colour_by_tag(r, tag.tag);
      continue;
    }

    // not selected, not hovered: set to default colour?
    if (const auto* preferred_colour = r.try_get<DefaultColour>(e))
      sc.colour = engine::SRGBToLinear(preferred_colour->colour);
    // else if (const auto* tag = r.try_get<TagComponent>(e))
    //   sc.colour = get_lin_colour_by_tag(r, tag->tag);
  }

  // Set colour for anything hovered
  // const auto& view = r.view<SpriteComponent, HoveredComponent, ChangeColourOnHoverComponent, TagComponent>(
  //   entt::exclude<WaitForInitComponent>);
  // for (const auto& [e, sc, hovered, hovered_col, tag] : view.each()) {
  // }
}

} // namespace game2d