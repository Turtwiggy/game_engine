#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"

namespace game2d {

void
update_ux_hoverable_change_colour_system(entt::registry& r)
{
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  const auto& view =
    r.view<SpriteComponent, HoveredComponent, ChangeColourOnHoverComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto [entity, sc, hovered, hovered_col] : view.each()) {
    sc.colour = *colours.lin_hit;
  }
}

} // namespace game2d