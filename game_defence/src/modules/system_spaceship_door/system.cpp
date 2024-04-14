#include "system.hpp"

#include "components.hpp"

namespace game2d {

void
update_spaceship_door_system(entt::registry& r)
{
  const auto& view = r.view<SpaceshipDoorComponent>();
  for (const auto& [e, door] : view.each()) {
    //
  }
}

} // namespace game2d