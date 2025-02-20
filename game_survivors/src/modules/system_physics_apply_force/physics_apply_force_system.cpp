#include "physics_apply_force_system.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"

#include <SDL2/SDL_log.h>
#include <box2d/b2_math.h>
#include <glm/ext/quaternion_geometric.hpp>

#if defined(_MSC_VER)
#include <optick.h>
#endif

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
    // return b_body->GetLinearVelocity() + req.speed * dir;
    return req.speed * dir;

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
#endif

void
update_physics_apply_force_system(entt::registry& r)
{
#if defined(_MSC_VER)
  OPTICK_EVENT();
#endif

  // Force to DynamicTarget
  {
    const auto& view =
      r.view<PhysicsBodyComponent, TransformComponent, const ApplyForceToDynamicTarget, const PhysicsDynamicTarget>();
    for (const auto& [e, body_c, t_c, req_c, target_c] : view.each()) {

      // check your target hasn't died
      const auto target_e = target_c.target;
      if (target_e == entt::null || !r.valid(target_e)) {
        SDL_Log("Target invalid. Should retarget.");
        r.remove<PhysicsDynamicTarget>(e);
        continue;
      }

      const auto& b_body = r.get<PhysicsBodyComponent>(target_c.target).body;

      auto& a_body = body_c.body;
      const auto cur_vel = a_body->GetLinearVelocity();

      // Compute the desired velocity of your spaceship.
      const b2Vec2 desired_vel = calculate_desired_velocity(a_body, b_body, req_c);

      // Calculate the velocity error
      const float mass = body_c.body->GetMass();
      const float proportional_gain = 10.0f;
      const b2Vec2 vel_err = desired_vel - cur_vel;
      const b2Vec2 force = mass * (proportional_gain * vel_err);

      // Could also clamp force here...
      // to stop exTrEmE forces

      // Apply the force
      a_body->ApplyForceToCenter(force, true);

      // Set ship angle as velocity
      const auto& vel = body_c.body->GetLinearVelocity();
      const float angle = engine::dir_to_angle_radians({ vel.x, vel.y });
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
      const float proportional_gain = 1.0f;
      const b2Vec2 force = proportional_gain * vel_err;

      // Apply the force
      body_c.body->ApplyForceToCenter(force, true);

      // Set ship angle as velocity
      const auto& vel = body_c.body->GetLinearVelocity();
      const float angle = engine::dir_to_angle_radians({ vel.x, vel.y }) + engine::PI;
      body_c.body->SetTransform(body_c.body->GetPosition(), angle);
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

      const auto& you_body = body_c.body;
      const auto& tgt_body = r.get<PhysicsBodyComponent>(tgt_c.target).body;

      const auto you_pos = glm::vec2{ you_body->GetPosition().x, you_body->GetPosition().y };
      const auto tgt_pos = glm::vec2{ tgt_body->GetPosition().x, tgt_body->GetPosition().y };

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
      const float speed = speed_c.base_speed; // m/s
      const float mass = body_c.body->GetMass();
      const b2Vec2 vel = speed * b2Vec2{ flank_nrm_dir.x, flank_nrm_dir.y };
      const b2Vec2 impulse = mass * vel;
      body_c.body->ApplyLinearImpulseToCenter(impulse, true);

#if defined(_DEBUG)
      debug_instances.push_back({
        .pos = meters_to_pixels(you_body->GetPosition()),
        .normal = cur_dir_normal,
        .nrm_dir = nrm_dir,
        .per_approach_dir = approach_dir,
        .angle_error_non_abs = angle_error_non_abs,
        .angle_error_adj = angle_error_adj,
        .distance = d,
        .midpoint = meters_to_pixels({ midpoint.x, midpoint.y }),
        .flankpoint = meters_to_pixels({ flankpoint.x, flankpoint.y }),
      });
#endif
    }
  }
}

void
update_physics_apply_force_debug_ui(entt::registry& r)
{
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
#endif
}

} // namespace game2d