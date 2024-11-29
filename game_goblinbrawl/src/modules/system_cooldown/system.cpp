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

    if (cd.time > 0.0f)
      cd.time -= dt;

    cd.time = std::max(0.0f, cd.time);
  }
};

} // namespace game2d