#include "engine/physics/physics_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"

#if defined(_MSC_VER)
#include <optick.h>
#endif

namespace game2d {

void
update_physics_system(entt::registry& r, const uint64_t ms_dt)
{
#if defined(_MSC_VER)
  OPTICK_EVENT();
#endif

  const auto physics_e = get_first<SINGLE_Physics>(r);
  if (physics_e == entt::null)
    return;
  const auto& physics = get_first_component<SINGLE_Physics>(r);

  // update world
  {
    const auto dt = ms_dt / 1000.0f;
    const int vel_iterations = 8;
    const int pos_iterations = 3;
    physics.world->Step(dt, vel_iterations, pos_iterations);
  }

  // update renderer
  {
    // const auto& view = r.view<const PhysicsBodyComponent, TransformComponent>(entt::exclude<SeparateTransformAndAABB>);
    const auto& view = r.view<const PhysicsBodyComponent, TransformComponent>();
    for (const auto& [e, body_c, transform_c] : view.each()) {
      const b2Vec2& position = body_c.body->GetPosition();
      transform_c.position.x = position.x;
      transform_c.position.y = position.y;

      // don't update the sprite scale.
      // when the physics object rotates,
      // the aabb grows which makes the sprite look like it's "jumping"
      // const auto& size = get_size(r, e);
      // transform_c.scale.x = size.x;
      // transform_c.scale.y = size.y;
    }
  }
  {
    const auto& view =
      r.view<const PhysicsBodyComponent, TransformComponent, const SetTransformRotationBasedOnPhysicsBody>();
    for (const auto& [e, body_c, transform_c, req_c] : view.each()) {
      const float angle = body_c.body->GetAngle();
      transform_c.rotation_radians.z = angle;
    }
  }
  {
    const auto& view =
      r.view<const PhysicsBodyComponent, TransformComponent, const SetTransformRotationBasedOnPhysicsVelocity>();
    for (const auto& [e, body_c, transform_c, req_c] : view.each()) {
      const auto vel = body_c.body->GetLinearVelocity();
      const auto dir = engine::normalize_safe({ vel.x, vel.y });
      const auto angle = engine::dir_to_angle_radians({ dir.x, dir.y });
      transform_c.rotation_radians.z = angle;
    }
  }
}

} // namespace game2d