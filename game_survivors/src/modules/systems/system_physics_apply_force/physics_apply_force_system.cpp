#include "pch.hpp"

#include "physics_apply_force_system.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"

namespace game2d {

// vertex at vx, vy
// x-axis roots at r1, r2
auto parabola = [](float x, float r1, float r2, float vx, float vy) -> float {
  // r1 = -10, r2 = 10, vx = 0, vy = 1;
  const auto scale = vy / ((vx - r1) * (vx - r2));
  const auto y = scale * (x - r1) * (x - r2);
  return y;
};

glm::vec2
apply_inside_out_force(const ApplyForceToDynamicTarget& req, glm::vec2 nrm_dir, float distance)
{
  const auto counter_dir = -nrm_dir;
  const auto distance_from_orbit_ring = glm::abs(req.distance_to_reduce_thrust - distance);

  // note: clamp the distance from orbit_ring to a curve
  // if distance_from_orbit_ring is negative, you're outside.
  const float error_dst = 3;
  const auto x = glm::clamp(distance_from_orbit_ring, -error_dst, error_dst);

  // scale dst to -1 to 1, e.g.
  // where -5 is y=0
  // where 0 is y=1
  // where 5 is y=0
  const float modified_x = parabola(x, -error_dst, error_dst, 0, 1);

  // when distance is 0, apply no force
  // when distance is at boundary, apply max force val is (1.0)
  // therefore, take the 1.0 - modified_x
  const float inv_modified_x = 1.0 - modified_x;

  return counter_dir * inv_modified_x;
};

glm::vec2
calculate_desired_velocity(entt::registry& r,
                           b2BodyId a_body,
                           entt::entity b_e,
                           const ActorSpeedComponent& speed_c,
                           const ApplyForceToDynamicTarget& req)
{
  const auto a_pos = b2Body_GetPosition(a_body);
  const auto b_pos = pixels_to_meters(get_position(r, b_e));
  b2Vec2 dir_b2d = b_pos - a_pos;
  const auto raw_dir = glm::vec2{ dir_b2d.x, dir_b2d.y };
  const auto nrm_dir = engine::normalize_safe(raw_dir);

  // full-speed ahead!
  if (!req.reduce_thrusters)
    return speed_c.current_speed * nrm_dir;

  // full-speed ahead!
  const float distance = glm::length(raw_dir);
  if (distance > req.distance_to_reduce_thrust)
    return speed_c.current_speed * nrm_dir;

  // Adjust the desired vel to account for target's velocity,
  // reduce speed the closer to the target you get
  const float percent = glm::clamp((distance / req.distance_to_reduce_thrust), 0.0f, 1.0f);
  const glm::vec2 reduced_vel = percent * speed_c.current_speed * nrm_dir;

  // try adding perpendcular vel to make it orbit
  // the closer you get, the stronger the orbit vel becomes
  // in order to try and prevent crash
  glm::vec2 orbit_vel{ 0.0f, 0.0f };
  if (req.orbit) {
    const auto perp = glm::vec2{ -nrm_dir.y, nrm_dir.x };
    orbit_vel = (1 - percent) * speed_c.current_speed * perp;

    // If you're too close, apply a push-out force.
    // orbit_vel += 0.1f * req.speed * apply_inside_out_force(req, nrm_dir, distance);
  }

  // If your target is a physics object, adjust for their velocity
  glm::vec2 b_vel{ 0, 0 };
  if (auto* b_body_c = r.try_get<PhysicsBodyComponent>(b_e)) {
    auto b2_b_vel = b2Body_GetLinearVelocity(b_body_c->bodyId);
    b_vel = { b2_b_vel.x, b2_b_vel.y };
  }

  return b_vel + reduced_vel + orbit_vel;
};

#if defined(_DEBUG)

struct DebugApproachDir
{
  glm::vec2 pos;
  glm::vec2 normal;
  glm::vec2 nrm_dir;
  glm::vec2 per_approach_dir;
  float angle_error_non_abs;
  float angle_error_adj;
  float distance;
  glm::vec2 midpoint;
  glm::vec2 flankpoint;
};
static std::vector<DebugApproachDir> debug_instances;

struct DebugVelocityError
{
  glm::vec2 tgt_vel;
  glm::vec2 cur_vel;
};
static std::vector<DebugVelocityError> debug_vel_instances;

#endif

void
update_physics_apply_force_system(entt::registry& r)
{
  // Force to DynamicTarget
#if defined(_DEBUG)
  debug_vel_instances.clear();
#endif
  {
    const auto& view = r.view<PhysicsBodyComponent,
                              TransformComponent,
                              const ActorSpeedComponent,
                              const ApplyForceToDynamicTarget,
                              const PhysicsDynamicTarget>();
    for (const auto& [e, body_c, t_c, speed_c, req_c, target_c] : view.each()) {

      // check your target hasn't died
      const auto target_e = target_c.target;
      if (target_e == entt::null || !r.valid(target_e)) {
        SDL_Log("Target invalid. Should retarget.");
        r.remove<PhysicsDynamicTarget>(e);
        continue;
      }

      const auto b_ent = target_c.target;
      const auto cur_vel = b2Body_GetLinearVelocity(body_c.bodyId);

      // Compute the desired velocity of your spaceship.
      const auto desired_vel = calculate_desired_velocity(r, body_c.bodyId, b_ent, speed_c, req_c);

      // debug_vel_instances.push_back(DebugVelocityError{
      //   .tgt_vel = { desired_vel.x, desired_vel.y },
      //   .cur_vel = { cur_vel.x, cur_vel.y },
      // });

      // Calculate the velocity error
      const float mass = b2Body_GetMass(body_c.bodyId);
      const float rate_of_change = 1.0f;
      const b2Vec2 vel_err = b2Vec2{ desired_vel.x, desired_vel.y } - cur_vel;
      const b2Vec2 force = mass * (rate_of_change * vel_err);

      // Could also clamp force here...
      // to stop exTrEmE forces

      // Apply the force
      b2Body_ApplyForceToCenter(body_c.bodyId, force, true);
    }
  }

  // Force in Direction
  {
    const auto& view = r.view<const ApplyForceInDirectionComponent, PhysicsBodyComponent>();
    for (const auto& [e, dir_c, body_c] : view.each()) {
      const auto cur_vel = b2Body_GetLinearVelocity(body_c.bodyId);
      const auto vel_err = b2Vec2{ dir_c.tgt_vel.x, dir_c.tgt_vel.y } - cur_vel;

      // how much force to apply?
      const float proportional_gain = 1.0f;
      const b2Vec2 force = proportional_gain * vel_err;

      // Apply the force
      b2Body_ApplyForceToCenter(body_c.bodyId, force, true);

      // Set ship angle as velocity
      const auto& vel = b2Body_GetLinearVelocity(body_c.bodyId);
      const float angle = engine::dir_to_angle_radians({ vel.x, vel.y }) + engine::PI;
      b2Body_SetTransform(body_c.bodyId, b2Body_GetPosition(body_c.bodyId), b2MakeRot(angle));
    }
  }

  // Force to Target via Angle
  // note: could have a flanking manager assign angles out to these enemies
  // this will mean that enemies will approach the player from all sorts of different angles.
  // if far... arc angles
  // if close... go direct!
#if defined(_DEBUG)
  debug_instances.clear();
#endif
  {
    const auto& view = r.view<const ApplyForceToApproachTargetFromAngle,
                              PhysicsBodyComponent,
                              const PhysicsDynamicTarget,
                              const ActorSpeedComponent>();
    for (const auto& [e, req_c, body_c, tgt_c, speed_c] : view.each()) {

      // check your target hasn't died
      const auto target_e = tgt_c.target;
      if (target_e == entt::null || !r.valid(target_e)) {
        SDL_Log("Target invalid. Should retarget.");
        r.remove<PhysicsDynamicTarget>(e);
        continue;
      }

      const auto you_body = body_c.bodyId;
      const auto tgt_body = r.get<PhysicsBodyComponent>(tgt_c.target).bodyId;

      const auto apos = b2Body_GetPosition(you_body);
      const auto bpos = b2Body_GetPosition(tgt_body);
      const auto you_pos = glm::vec2(apos.x, apos.y);
      const auto tgt_pos = glm::vec2(bpos.x, bpos.y);

      const auto raw_dir = you_pos - tgt_pos;
      const auto nrm_dir = engine::normalize_safe(raw_dir);
      const auto midpoint = you_pos - glm::vec2(0.5) * raw_dir;

      const auto approach_ang = req_c.angle_rad;
      const auto approach_dir = engine::angle_radians_to_direction(approach_ang);

      const float cur_angle = engine::dir_to_angle_radians(nrm_dir);
      const float per_angle = engine::dir_to_angle_radians(approach_dir);
      const float angle_error_non_abs = cur_angle - per_angle;
      const float angle_error = glm::abs(cur_angle - per_angle);
      const float angle_error_adj = angle_error / 2.0f;

      // point normal in the shortest direction
      auto cur_dir_normal_cw = engine::normalize_safe({ -nrm_dir.y, nrm_dir.x });
      auto cur_dir_normal = cur_dir_normal_cw;

      // take the shorter path
      if (angle_error_non_abs > 0.0f)
        cur_dir_normal *= -1;

      // offset the midpoint via normal.
      const auto d = glm::length(raw_dir);
      const float amplitude_meters = d * 0.5f;

      auto flankpoint = midpoint + amplitude_meters * cur_dir_normal * sin(angle_error_adj);
      // if you're less than one meter, just go direct.
      if (d < 1.0)
        flankpoint = tgt_pos;

      // start flankin'
      const glm::vec2 flank_raw_dir = flankpoint - you_pos;
      const glm::vec2 flank_nrm_dir = engine::normalize_safe(flank_raw_dir);
      const float speed = speed_c.current_speed; // m/s
      const float mass = b2Body_GetMass(body_c.bodyId);
      const b2Vec2 vel = speed * b2Vec2{ flank_nrm_dir.x, flank_nrm_dir.y };
      const b2Vec2 impulse = mass * vel;
      b2Body_ApplyLinearImpulseToCenter(body_c.bodyId, impulse, true);

#if defined(_DEBUG)
      // debug_instances.push_back({
      //   .pos = meters_to_pixels(you_body->GetPosition()),
      //   .normal = cur_dir_normal,
      //   .nrm_dir = nrm_dir,
      //   .per_approach_dir = approach_dir,
      //   .angle_error_non_abs = angle_error_non_abs,
      //   .angle_error_adj = angle_error_adj,
      //   .distance = d,
      //   .midpoint = meters_to_pixels({ midpoint.x, midpoint.y }),
      //   .flankpoint = meters_to_pixels({ flankpoint.x, flankpoint.y }),
      // });
#endif
    }
  }
}

void
update_physics_apply_force_debug_ui(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
#if defined(_DEBUG)
  const auto& ds = debug_instances;

  // ImGui::Begin("UpdatePhysicsDebug");

  for (const auto& d : ds) {
    // ImGui::Separator();
    // ImGui::Text("current_approach_dir: %f %f", d.nrm_dir.x, d.nrm_dir.y);
    // ImGui::Text("perfect_approach_dir: %f %f", d.per_approach_dir.x, d.per_approach_dir.y);
    // ImGui::Text("Angle error NonABS: %f", d.angle_error_non_abs);
    // ImGui::Text("Angle error Adj: %f", d.angle_error_adj);
    // ImGui::Text("Distance: %f", d.distance);

    {
      Sprite debug_s;
      debug_s.pos = d.midpoint;
      debug_s.sprite = "EMPTY";
      debug_s.size = { 8, 8 };
      debug_s.col = { 1.0f, 0.0f, 0.0f, 1.0f };
      draw_sprite(r, debug_s);
    }
    {
      Sprite debug_s;
      debug_s.pos = d.flankpoint;
      debug_s.sprite = "EMPTY";
      debug_s.size = { 4, 4 };
      debug_s.col = { 1.0f, 1.0f, 0.0f, 1.0f };
      draw_sprite(r, debug_s);
    }

    const auto b = d.pos + (50.0f * d.normal);
    const LineInfo line_info = generate_line(d.pos, b, 2);
    {
      Sprite debug_s;
      debug_s.pos = line_info.position;
      debug_s.size = line_info.scale;
      debug_s.z_rotation = line_info.rotation;
      debug_s.sprite = "EMPTY";
      debug_s.col = { 1.0f, 1.0f, 1.0f, 1.0f };
      draw_sprite(r, debug_s);
    }
  }
  // ImGui::End();

  // ImGui::Begin("DebugVel");
  // for (const auto& d : debug_vel_instances) {
  //   ImGui::Text("CurVel %f %f", d.cur_vel.x, d.cur_vel.y);
  //   ImGui::Text("TgtVel %f %f", d.tgt_vel.x, d.tgt_vel.y);
  // }
  // ImGui::End();

#endif
}

} // namespace game2d