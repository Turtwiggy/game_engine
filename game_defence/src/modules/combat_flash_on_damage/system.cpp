#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "resources/colours.hpp"

namespace game2d {

void
update_flash_sprite_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  const auto& view =
    r.view<SpriteComponent, FlashOnDamageComponent, const EntityTypeComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto [entity, sc, flash, etc] : view.each()) {

    if (!flash.started) {
      flash.started = true;
      sc.colour = *colours.lin_hit;
    }

    flash.milliseconds_left -= milliseconds_dt;

    if (flash.milliseconds_left <= 0) {

      // todo: set back to original colour
      sc.colour = *colours.lin_primary;

      r.remove<FlashOnDamageComponent>(entity);
    }
  }
};

} // namespace game2d