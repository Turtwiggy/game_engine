#include "system.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/system_overworld_change_direction/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <box2d/b2_math.h>
#include <fmt/core.h>

namespace game2d {

b2Vec2
calculate_desired_velocity(b2Body* a_body, b2Body* b_body, const ApplyForceToDynamicTarget& req)
{
  b2Vec2 dir = b_body->GetPosition() - a_body->GetPosition();

  // distance from target
  const float distance = dir.Length();
  dir.Normalize();

  // full-speed ahead!
  if (distance > req.distance_to_reduce_thrust || !req.reduce_thrusters)
    return b_body->GetLinearVelocity() + req.speed * dir;

  // Adjust the desired vel to account for target's velocity,
  // reduce speed the closer to the target you get
  const float percent = (distance / req.distance_to_reduce_thrust);
  const b2Vec2 reduced_vel = percent * req.speed * dir;

  // try adding perpendcular vel to make it orbit
  // the closer you get, the stronger the orbit vel becomes
  // in order to try and prevent crash
  b2Vec2 orbit_vel{ 0.0f, 0.0f };
  if (req.orbit) {
    const b2Vec2 perp = { -dir.y, dir.x };
    orbit_vel = (1 - percent) * req.speed * perp;
  }

  return b_body->GetLinearVelocity() + reduced_vel + orbit_vel;
};

void
update_physics_apply_force_system(entt::registry& r)
{
  // Force to DynamicTarget
  {
    const auto& view =
      r.view<PhysicsBodyComponent, TransformComponent, const ApplyForceToDynamicTarget, const DynamicTargetComponent>();
    for (const auto& [e, body_c, t_c, req_c, target_c] : view.each()) {
      const auto& b_body = r.get<PhysicsBodyComponent>(target_c.target).body;

      auto& a_body = body_c.body;
      const auto cur_vel = a_body->GetLinearVelocity();

      // Compute the desired velocity of your spaceship.
      const b2Vec2 desired_vel = calculate_desired_velocity(a_body, b_body, req_c);

      // Calculate the velocity error
      const float proportional_gain = 100.0f;
      const b2Vec2 vel_err = desired_vel - cur_vel;
      const b2Vec2 force = proportional_gain * vel_err;

      // Could also clamp force here...
      // to stop exTrEmE forces

      // Apply the force
      a_body->ApplyForceToCenter(force, true);

      // Set ship angle as velocity
      const auto& vel = body_c.body->GetLinearVelocity();
      const float angle = engine::dir_to_angle_radians({ vel.x, vel.y }) + engine::PI;
      body_c.body->SetTransform(body_c.body->GetPosition(), angle);
    }
  }

  // Force in Direction
  {
    const auto& view = r.view<const ApplyForceInDirectionComponent, PhysicsBodyComponent>();
    for (const auto& [e, dir_c, body_c] : view.each()) {
      const auto& cur_vel = body_c.body->GetLinearVelocity();
      const auto vel_err = b2Vec2{ dir_c.tgt_vel.x, dir_c.tgt_vel.y } - cur_vel;

      // how much force to apply?
      const float proportional_gain = 100.0f;
      const b2Vec2 force = proportional_gain * vel_err;

      // Apply the force
      body_c.body->ApplyForceToCenter(force, true);

      // Set ship angle as velocity
      const auto& vel = body_c.body->GetLinearVelocity();
      const float angle = engine::dir_to_angle_radians({ vel.x, vel.y }) + engine::PI;
      body_c.body->SetTransform(body_c.body->GetPosition(), angle);
    }
  }
}

} // namespace game2d