#include "system.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <box2d/b2_math.h>
#include <fmt/core.h>

namespace game2d {

b2Vec2
to_box2d(const glm::vec2& glmVec)
{
  return b2Vec2(glmVec.x, glmVec.y);
};

glm::vec2
to_glm(const b2Vec2& b2Vec)
{
  return glm::vec2(b2Vec.x, b2Vec.y);
};

b2Vec2
calculate_desired_velocity(b2Body* a_body, b2Body* b_body, const float speed)
{
  b2Vec2 dir = b_body->GetPosition() - a_body->GetPosition();

  // distance from target
  const float distance = dir.Length();

  dir.Normalize();

  float distance_to_stop_thrust = 200;

  // full-speed ahead!
  if (distance > distance_to_stop_thrust)
    return b_body->GetLinearVelocity() + speed * dir;

  // Adjust the desired vel to account for target's velocity,
  // reduce speed the closer to the target you get
  const float percent = (distance / distance_to_stop_thrust);
  const b2Vec2 reduced_vel = percent * speed * dir;

  // HACK: try adding perpendcular vel to make it orbit
  // the closer you get, the stronger the orbit vel becomes
  // in order to try and prevent crash
  const b2Vec2 perp = { -dir.y, dir.x };
  const b2Vec2 orbit_vel = (1 - percent) * speed * perp;

  return b_body->GetLinearVelocity() + reduced_vel + orbit_vel;
};

void
update_physics_apply_force_system(entt::registry& r)
{
  const auto& view =
    r.view<PhysicsBodyComponent, TransformComponent, const ApplyForceToDynamicTarget, const DynamicTargetComponent>();
  for (const auto& [e, body_c, t_c, req_c, target_c] : view.each()) {
    auto& a_body = body_c.body;
    const auto& b_body = r.get<PhysicsBodyComponent>(target_c.target).body;

    const b2Vec2 a_vel = a_body->GetLinearVelocity();

    // Compute the desired velocity of your spaceship.
    const float speed = 100.0f;
    const b2Vec2 desired_vel = calculate_desired_velocity(a_body, b_body, speed);

    // Calculate the velocity error
    const float proportional_gain = 100.0f;
    const b2Vec2 vel_err = desired_vel - a_vel;
    const b2Vec2 force = proportional_gain * vel_err;

    // Could also clamp force here...
    // to stop exTrEmE forces

    // Apply the force
    a_body->ApplyForceToCenter(force, true);

    // Set ship angle as velocity
    const float angle = engine::dir_to_angle_radians(to_glm(body_c.body->GetLinearVelocity())) + engine::PI;
    body_c.body->SetTransform(body_c.body->GetPosition(), angle);
  }
}

} // namespace game2d