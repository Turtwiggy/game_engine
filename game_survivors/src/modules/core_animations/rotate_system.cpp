#include "rotate_system.hpp"

#include "engine/renderer/transform.hpp"
#include "rotate_components.hpp"

namespace game2d {

void
update_animation_rotate_system(entt::registry& r, const float dt)
{
  for (const auto& [e, rotate_c, t_c] : r.view<AnimationRotate, TransformComponent>().each()) {
    t_c.rotation_radians.z += dt * rotate_c.speed;
  }
}

} // namespace game2d