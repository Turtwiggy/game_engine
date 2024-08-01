#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_physics_system(entt::registry& r, const uint64_t ms_dt)
{
  const auto physics_e = get_first<SINGLE_Physics>(r);
  if (physics_e == entt::null)
    return;
  auto& physics = get_first_component<SINGLE_Physics>(r);

  // update world
  {
    const int vel_iterations = 8;
    const int pos_iterations = 3;
    physics.world->Step(ms_dt / 1000.0f, vel_iterations, pos_iterations);
  }

  // update renderer
  {
    const auto& view = r.view<PhysicsBodyComponent, TransformComponent>();
    for (const auto& [e, body_c, transform_c] : view.each()) {
      const b2Vec2& position = body_c.body->GetPosition();
      const float& angle = body_c.body->GetAngle();

      transform_c.position.x = position.x;
      transform_c.position.y = position.y;
      transform_c.rotation_radians.z = angle;

      // don't update the sprite scale.
      // when the physics object rotates,
      // the aabb grows which makes the sprite look like it's "jumping"
      // const auto& size = get_size(r, e);
      // transform_c.scale.x = size.x;
      // transform_c.scale.y = size.y;
    }
  }
}

} // namespace game2d