#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

namespace game2d {

void
update_flash_sprite_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const auto& view = r.view<SpriteComponent, RequestFlashComponent, TagComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, sc, request, tag] : view.each()) {

    if (request.milliseconds_left > 0)
      sc.colour = get_lin_colour_by_tag(r, "enemy_flash");

    else {
      sc.colour = get_lin_colour_by_tag(r, tag.tag);

      // done flashing
      r.remove<RequestFlashComponent>(e);
    }

    request.milliseconds_left -= milliseconds_dt;
  }
};

} // namespace game2d