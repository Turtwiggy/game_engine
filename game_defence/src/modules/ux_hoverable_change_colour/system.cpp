#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_ux_hoverable_change_colour_system(entt::registry& r)
{
  const auto& view =
    r.view<SpriteComponent, HoveredComponent, ChangeColourOnHoverComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto [entity, sc, hovered, hovered_col] : view.each()) {
    const auto colour = get_lin_colour_by_tag(r, "hovered");
    sc.colour = colour;
  }
}

} // namespace game2d