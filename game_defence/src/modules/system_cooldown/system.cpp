#include "system.hpp"

#include "components.hpp"
#include "engine/lifecycle/components.hpp"

namespace game2d {

void
update_cooldown_system(entt::registry& r, const uint64_t milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  const auto& view = r.view<CooldownComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, cd] : view.each()) {
    if (cd.time_left > 0.0f)
      cd.time_left -= dt;
    cd.on_cooldown = cd.time_left > 0.0f;
  }
};

} // namespace game2d