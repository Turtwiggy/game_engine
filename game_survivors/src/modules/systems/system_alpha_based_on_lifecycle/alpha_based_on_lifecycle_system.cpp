#include "alpha_based_on_lifecycle_system.hpp"

#include "alpha_based_on_lifecycle_components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/sprites/components.hpp"

namespace game2d {

void
update_alpha_based_on_lifecycle_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  {
    const auto& view = r.view<const SetAlphaBasedOnLifecycleComponent, SpriteComponent, const EntityTimedLifecycle>();
    for (const auto& [e, req_c, sprite_c, lifecycle_c] : view.each()) {
      const auto decrement = (lifecycle_c.milliseconds_alive_max - lifecycle_c.milliseconds_alive);

      float percent = decrement / (float)lifecycle_c.milliseconds_alive_max;
      percent = glm::clamp(percent, 0.4f, 1.0f); // low alpha turns the sprite black

      sprite_c.colour.a = percent;
    }
  }

  const auto& view = r.view<const SetColourOnLifecycleComponent, SpriteComponent, const EntityTimedLifecycle>();
  for (const auto& [e, req_c, sprite_c, lifecycle_c] : view.each()) {
    const auto decrement = (lifecycle_c.milliseconds_alive_max - lifecycle_c.milliseconds_alive);

    float percent = decrement / (float)lifecycle_c.milliseconds_alive_max;
    percent = glm::clamp(percent, 0.0f, 1.0f); // low alpha turns the sprite black

    // todo: lerp colours
    sprite_c.colour.r = percent;
    sprite_c.colour.g = percent;
    sprite_c.colour.b = percent;
  }
}

} // namespace game2d