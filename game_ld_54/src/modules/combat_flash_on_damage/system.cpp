#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "resources/colours.hpp"

namespace game2d {

void
update_flash_sprite_system(entt::registry& r, uint64_t milliseconds_dt)
{
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  const auto& view = r.view<SpriteColourComponent, FlashOnDamageComponent, const EntityTypeComponent>();
  for (const auto [entity, scc, flash, etc] : view.each()) {

    if (!flash.started) {
      flash.started = true;
      scc.colour = colours.lin_hit;
      continue;
    }

    flash.milliseconds_left -= milliseconds_dt;

    if (flash.milliseconds_left <= 0) {
      // set to original colour
      SpriteColourComponent spr = create_colour(colours, etc.type);
      scc.colour = spr.colour;
      r.remove<FlashOnDamageComponent>(entity);
    }
  }
};

} // namespace game2d