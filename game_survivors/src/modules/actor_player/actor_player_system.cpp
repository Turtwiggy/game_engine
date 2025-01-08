#include "modules/actor_player/actor_player_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/controller.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_mouse.h>
#include <SDL_scancode.h>
#include <box2d/b2_math.h>
#include <box2d/box2d.h>
#include <glm/glm.hpp>

#include <imgui.h>

namespace game2d {

// https://www.youtube.com/watch?v=LSNQuFEDOyQ
const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
  //
  return b + (a - b) * glm::exp(-decay * dt);
  //
};

void
update_movement_jetpack(entt::registry& r)
{
  const float rotation_speed = 1.5f;
  const float speed = 100.0f;

  const float max_force = 100.0f;
  const float proportional_gain = 10000.0f;
  const bool clamp_max_force = false;

  const auto& view = r.view<const InputComponent, const MovementJetpackComponent, PhysicsBodyComponent>();
  for (const auto& [e, input_c, movetype_c, body_c] : view.each()) {
    body_c.body->SetLinearDamping(1.0f);

    // rotate
    body_c.body->SetAngularVelocity(input_c.lx * rotation_speed);

    const float epsilon = 0.0001f;
    if (glm::abs(input_c.ly) - epsilon <= 0.0f)
      continue; // no input, dont move

    // -engine::HALF_PI so that the angle is not from the left side, but from the feet
    const float angle = body_c.body->GetAngle() - engine::HALF_PI;
    const auto dir = engine::angle_radians_to_direction(angle);
    const b2Vec2 tgt_vel = b2Vec2(dir.x * speed, dir.y * speed);

    const b2Vec2 cur_vel = body_c.body->GetLinearVelocity();
    const b2Vec2 vel_err = tgt_vel - cur_vel;
    b2Vec2 force = proportional_gain * vel_err;

    if (clamp_max_force) {
      float force_magnitude_sq = force.LengthSquared();
      float max_force_mag_sq = max_force * max_force;
      if (force_magnitude_sq > max_force_mag_sq) {
        float force_mag = glm::sqrt(force_magnitude_sq);
        force *= (max_force / force_mag);
      }
    }

    body_c.body->ApplyForceToCenter(force, true);
  }
};

void
update_movement_direct(entt::registry& r, const uint64_t ms_dt)
{
  const float dt = ms_dt / 1000.0f;

  const auto& view = r.view<const InputComponent, const MovementDirectComponent, PhysicsBodyComponent>();
  for (const auto& [e, input_c, movetype_c, body_c] : view.each()) {
    const glm::vec2 l_nrm_raw = { input_c.lx, input_c.ly };
    const glm::vec2 l_nrm_dir = engine::normalize_safe(l_nrm_raw);

    const glm::vec2 move_vel = (l_nrm_dir * body_c.base_speed);
    body_c.body->SetLinearVelocity({ move_vel.x, move_vel.y });

    //
    // set rot here as the body itself doesnt rotate with movement_direct
    // this functionality is equivilent to look_in_direction_of_movemen t
    //
    if (glm::length(l_nrm_dir) <= 0.0f)
      continue;
    const float speed = 25.0f; // higher number = faster to destination
    const float max_angle = 15.0f * engine::Deg2Rad;

    const float cur_angle = std::fmod(body_c.body->GetAngle(), 2.0f * engine::PI);
    const float new_angle = std::fmod(engine::dir_to_angle_radians(l_nrm_dir), 2.0f * engine::PI);

    return;
    // ensure in range [0, 2PI]
    const float wrapped_cur_angle = (cur_angle < 0.0f) ? cur_angle + 2.0f * engine::PI : cur_angle;
    const float wrapped_new_angle = (new_angle < 0.0f) ? new_angle + 2.0f * engine::PI : new_angle;
    // SDL_Log("wcur: %f wnew: %f", wrapped_cur_angle, wrapped_new_angle);

    // Calculate angle diff
    float angle_diff = wrapped_new_angle - wrapped_cur_angle;
    if (angle_diff > engine::PI)
      angle_diff -= 2.0f * engine::PI; // Take the shorter path (counterclockwise)
    else if (angle_diff < -engine::PI)
      angle_diff += 2.0f * engine::PI; // Take the shorter path (clockwise)

    float clamped_angle_diff = glm::clamp(angle_diff, -max_angle, max_angle);

    // Compute the target angle in [0, 2π]
    float tgt_angle = wrapped_cur_angle + clamped_angle_diff;
    tgt_angle = std::fmod(tgt_angle, 2.0f * engine::PI);
    if (tgt_angle < 0.0f)
      tgt_angle += 2.0f * engine::PI;

    const float fin_angle = exp_decay(wrapped_cur_angle, tgt_angle, speed, dt);

    body_c.body->SetTransform(body_c.body->GetPosition(), fin_angle);
  }
};

void
update_movement_asteroids(entt::registry& r, uint64_t ms_dt)
{
  const auto& view =
    r.view<const InputComponent, const MovementAsteroidsComponent, PhysicsBodyComponent, const TransformComponent>();
  for (const auto& [e, input_c, movetype_c, body_c, transform_c] : view.each()) {

    // rotation_speed: how fast ship can look left/right
    // speed: how fast ship travels
    const float rotation_speed = 1.5f;
    const float speed = 100.0f;

    // A constant that determines how strongly the force is applied in response to the velocity error.
    // A higher value means more aggressive correction.
    const float max_force = 1000.0f;
    const float proportional_gain = 10000.0f;
    const bool clamp_max_force = false;

    // Choosing Between Torque and Angular Impulse:
    // Torque:
    // Applies a continuous force causing gradual rotation.
    // Good for simulating forces like engines or motors.
    // Angular Impulse:
    // Applies an instantaneous change in rotational velocity.
    // Useful for sudden rotational effects, like impacts or quick spins.
    if (movetype_c.able_to_change_dir)
      body_c.body->SetAngularVelocity(input_c.lx * rotation_speed);

    if (movetype_c.able_to_change_thrust) {
      // if (input_c.ly > 0)
      //   movetype_c.thrust -= thrust_change * (ms_dt / 1000.0f);
      // if (input_c.ly < 0)
      //   movetype_c.thrust += thrust_change * (ms_dt / 1000.0f);
    }
    // movetype_c.thrust = glm::min(movetype_c.thrust, 200.0f);
    // movetype_c.thrust = glm::max(movetype_c.thrust, 1.0f);

    const auto dir = engine::angle_radians_to_direction(transform_c.rotation_radians.z);
    const b2Vec2 tgt_vel = b2Vec2(dir.x * speed, dir.y * speed);

    /*
    When aiming to achieve and maintain a specific velocity for a dynamic object (like a spaceship)
    you should use a control approach that adjusts the force applied based on the difference between the current velocity
    and the desired velocity.
    This is typically done using a form of proportional control,
    which is a fundamental concept in control systems.
    */
    const b2Vec2 cur_vel = body_c.body->GetLinearVelocity();
    const b2Vec2 vel_err = tgt_vel - cur_vel;
    b2Vec2 force = proportional_gain * vel_err;

    // Clamp the force magnitude to avoid excessive values.
    // Game-feel wise, if clamp is yes:
    // ship feels more "physics-y", because it takes a little longer to turn
    // Game-feel wise, if clamp is no:
    // ship is more responsive beause large force values can be applied.
    // Kinda feels more like a car than a thruster-powered spaceship
    if (clamp_max_force) {
      float force_magnitude_sq = force.LengthSquared();
      float max_force_mag_sq = max_force * max_force;
      if (force_magnitude_sq > max_force_mag_sq) {
        float force_mag = glm::sqrt(force_magnitude_sq);
        force *= (max_force / force_mag);
      }
    }

    /*
      For more sophisticated control, you might consider implementing a PID
      (Proportional - Integral - Derivative) controller,
      which takes into account not only the current error(proportional control)
      but also the accumulated error over time(integral control) and
      the rate of change of the error(derivative control).
      This can provide smoother and more stable control in some cases.
    */

    body_c.body->ApplyForceToCenter(force, true);
  }
};

void
update_player_controller_system(entt::registry& r, const uint64_t milliseconds_dt, const glm::ivec2& mouse_pos)
{
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  int controllers_used = 0;

  const auto& view = r.view<InputComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, i] : view.each()) {
    //
    i.lx = 0.0f;
    i.ly = 0.0f;
    i.rx = 0.0f;
    i.ry = 0.0f;
    i.shoot = false;

    // set rx based on mouse input if selected
    if (const auto* selected_c = r.try_get<SelectedComponent>(e)) {
      const auto dir = glm::vec2{ mouse_pos.x, mouse_pos.y } - get_position(r, e);
      i.rx += dir.x;
      i.ry += dir.y;
      i.shoot = get_mouse_lmb_press();
      i.ly += get_key_held(input_c, SDL_SCANCODE_W) ? -1.0f : 0.0f;
      i.ly += get_key_held(input_c, SDL_SCANCODE_S) ? 1.0f : 0.0f;
      i.lx += get_key_held(input_c, SDL_SCANCODE_A) ? -1.0f : 0.0f;
      i.lx += get_key_held(input_c, SDL_SCANCODE_D) ? 1.0f : 0.0f;
    }

    if (const auto* controller_c = r.try_get<ControllerComponent>(e)) {
      if (controllers_used < int(input_c.controllers.size())) {

        // todo: map plugged in controller idxs to player
        // todo: dont just use idx 0
        auto* controller = input_c.controllers[0];

        i.lx += get_axis_01(controller, controller_c->c_left_stick_x);
        i.ly += get_axis_01(controller, controller_c->c_left_stick_y);
      }
      // else {
      //   SDL_Log("Not enough controllers plugged in...");
      // }
      controllers_used++;
    }
  }
};

void
fixed_update_player_controller_system(entt::registry& r, const uint64_t ms_dt, const glm::ivec2& mouse_pos)
{
  // What happens if multiple fixedupdate() before?

  update_movement_direct(r, ms_dt);

  // update_movement_asteroids(r, ms_dt);
  // update_movement_jetpack(r);
};

} // namespace game2d
