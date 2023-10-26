#include "wiggle_up_and_down.hpp"

#include "components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_wiggle_up_and_down_system(entt::registry& r, float dt)
{
  const auto& view = r.view<WiggleUpAndDown, TransformComponent>();
  for (const auto& [entity, wiggle, transform] : view.each()) {

    static float amplitude = 2.0f;
    static float frequency = 5.0f;

    transform.position.y = wiggle.base_position.y + (sin((wiggle.time + wiggle.offset) * frequency) * amplitude);
    wiggle.time += dt;
  }
}

} // namespace game2d