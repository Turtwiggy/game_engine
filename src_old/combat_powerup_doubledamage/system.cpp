#include "system.hpp"

#include "components.hpp"

namespace game2d {

void
update_combat_powerup_doubledamage_system(entt::registry& r, float dt)
{
  const auto& view = r.view<PowerupDoubleDamage>();
  for (const auto& [e, powerup] : view.each()) {
    //
    powerup.timeleft -= dt;
    if (powerup.timeleft <= 0.0F)
      r.remove<PowerupDoubleDamage>(e);
  }
};

} // namespace game2d