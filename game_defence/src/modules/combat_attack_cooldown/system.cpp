#include "system.hpp"

#include "components.hpp"
#include "modules/lifecycle/components.hpp"

namespace game2d {

void
update_attack_cooldown_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<AttackCooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, cd] : view.each()) {
    if (cd.time_between_attack_left > 0.0f)
      cd.time_between_attack_left -= dt;
    cd.on_cooldown = cd.time_between_attack_left > 0.0f;
  }
};

} // namespace game2d