#include "modules/systems/show_tiles_in_range.hpp"
#include "modules/actor_player/components.hpp"

namespace game2d {

void
update_show_tiles_in_range_system(entt::registry& r)
{
  const auto& view = r.view<PlayerComponent>();
  for (const auto& [e, player_c] : view.each()) {
    //
  }
}

} // namespace game2d