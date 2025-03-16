#include "enemy_swarmlord_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_spawner/spawner_system.hpp"

namespace game2d {

void
update_enemy_swarmlord_system(entt::registry& r)
{
  const auto view = r.view<CooldownComponent, SwarmLordComponent>();
  for (const auto& [e, cooldown_c, swarmlord_c] : view.each()) {
    if (cooldown_c.time > 0.0f)
      continue;
    reset_cooldown(cooldown_c);

    // spawn some jellyfish innit
    const int minion_hp = 10;
    auto minion_e = spawn_enemy(r, "actor_enemy_swarmlord_minion", minion_hp);
    set_position(r, minion_e, get_position(r, e));
  }
};

} // namespace game2d