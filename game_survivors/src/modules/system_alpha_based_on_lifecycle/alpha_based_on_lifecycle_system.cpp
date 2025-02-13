#include "alpha_based_on_lifecycle_system.hpp"

#include "alpha_based_on_lifecycle_components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/sprites/components.hpp"

namespace game2d {

void
update_alpha_based_on_lifecycle_system(entt::registry& r)
{
  const auto& view = r.view<SetAlphaBasedOnLifecycleComponent, SpriteComponent, const EntityTimedLifecycle>();
  for (const auto& [e, req_c, sprite_c, lifecycle_c] : view.each()) {
    const auto decrement = (lifecycle_c.milliseconds_alive_max - lifecycle_c.milliseconds_alive);

    float percent = decrement / (float)lifecycle_c.milliseconds_alive_max;
    percent = glm::clamp(percent, 0.4f, 1.0f); // low alpha turns the sprite black

    sprite_c.colour.a = percent;
  }
}

} // namespace game2d