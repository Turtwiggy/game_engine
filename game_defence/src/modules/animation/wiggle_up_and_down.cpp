#include "wiggle_up_and_down.hpp"

#include "components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_wiggle_up_and_down_system(entt::registry& r, float dt)
{
  const auto& view = r.view<WiggleUpAndDown, TransformComponent>(entt::exclude<PhysicsBodyComponent>);
  for (const auto& [entity, wig, transform] : view.each()) {
    transform.position.y = wig.base_position.y + (sin((wig.time + wig.offset) * wig.frequency) * wig.amplitude);
    wig.time += dt;
  }
}

} // namespace game2d