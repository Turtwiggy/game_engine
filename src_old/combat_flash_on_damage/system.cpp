#include "system.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/sprites/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/ux_hoverable/components.hpp"


#include <fmt/core.h>

namespace game2d {

void
update_flash_sprite_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const auto& view =
    r.view<SpriteComponent, RequestFlashComponent, const DefaultColour>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, sc, req, default_c] : view.each()) {

    if (req.milliseconds_left > 0 && !req.flashing) {
      req.flashing = true;
      set_colour(r, e, get_srgb_colour_by_tag(r, "enemy_flash"));

      // fmt::println("flashing...");
    }

    if (req.milliseconds_left <= 0) {
      // fmt::println("done flashing...");
      set_colour(r, e, default_c.colour);
      r.remove<RequestFlashComponent>(e); // done flashing
    }

    req.milliseconds_left -= static_cast<int>(milliseconds_dt);
  }
};

} // namespace game2d