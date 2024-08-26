#include "modules/actor_player/system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/fixed_update.hpp"
#include "lifecycle/components.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/system_sprint/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <box2d/b2_math.h>
#include <box2d/box2d.h>
#include <fmt/core.h>
#include <glm/glm.hpp>

namespace game2d {

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

    const auto dir = engine::angle_radians_to_direction(body_c.body->GetAngle());
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
}

void
update_movement_direct(entt::registry& r)
{
  const auto& view = r.view<const InputComponent, const MovementDirectComponent, PhysicsBodyComponent>();
  for (const auto& [e, input_c, movetype_c, body_c] : view.each()) {
    const glm::vec2 l_nrm_raw = { input_c.lx, input_c.ly };
    const glm::vec2 l_nrm_dir = engine::normalize_safe(l_nrm_raw);

    const glm::vec2 move_vel = (l_nrm_dir * body_c.base_speed);
    body_c.body->SetLinearVelocity({ move_vel.x, move_vel.y });
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
}

void
update_player_controller_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const auto& finputs = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finputs.history.at(finputs.fixed_tick);

  const auto& view = r.view<InputComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, input] : view.each()) {

    const auto* keyboard = r.try_get<KeyboardComponent>(entity);
    const auto* controller = r.try_get<ControllerComponent>(entity);

    // const auto l = [](const InputEvent& e) { return (e.type == InputType::keyboard && e.state == InputState::release); };
    // const bool release_key = std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
    // if (release_key)
    //   int k = 1;

    if (keyboard) {
      input.shoot_release = fixed_input_mouse_release(inputs, SDL_BUTTON_LEFT);
      input.sprint_release = fixed_input_keyboard_release(inputs, keyboard->sprint);
    }

    if (controller) {
      // if (input.shoot)
      //   input.shoot_release |= fixed_input_controller_axis_held(inputs, controller->c_right_trigger) <= 0.1f;
      // if (input.sprint)
      //   input.sprint_release |= fixed_input_controller_axis_held(inputs, controller->c_left_trigger) <= 0.1f;
    }

    // reset
    input.lx = 0.0f;
    input.ly = 0.0f;
    input.rx = 0.0f;
    input.ry = 0.0f;
    input.shoot = false;
    input.pickup = false;
    input.drop = false;
    input.sprint = false;

    // rx via mouse
    // const auto dir = transform.position - cursor_transform.position;
    // glm::vec2 r_nrm_dir = { dir.x, dir.y };
    // if (r_nrm_dir.x != 0.0f || r_nrm_dir.y != 0.0f)
    //   r_nrm_dir = glm::normalize(r_nrm_dir);
    // input.rx += r_nrm_dir.x;
    // input.ry += r_nrm_dir.y;

    if (keyboard) {
      input.ly += fixed_input_keyboard_held(inputs, keyboard->W) ? -1 : 0;
      input.ly += fixed_input_keyboard_held(inputs, keyboard->S) ? 1 : 0;
      input.lx += fixed_input_keyboard_held(inputs, keyboard->A) ? -1 : 0;
      input.lx += fixed_input_keyboard_held(inputs, keyboard->D) ? 1 : 0;
      input.shoot |= fixed_input_mouse_press(inputs, SDL_BUTTON_LEFT);
      input.pickup |= fixed_input_keyboard_press(inputs, keyboard->pickup);
      input.drop |= fixed_input_keyboard_press(inputs, keyboard->drop);
      input.sprint |= fixed_input_keyboard_press(inputs, keyboard->sprint);
    }

    if (controller) {
      input.lx += fixed_input_controller_axis_held(inputs, controller->c_left_stick_x);
      input.ly += fixed_input_controller_axis_held(inputs, controller->c_left_stick_y);
      input.rx += fixed_input_controller_axis_held(inputs, controller->c_right_stick_x);
      input.ry += fixed_input_controller_axis_held(inputs, controller->c_right_stick_y);
      input.shoot |= fixed_input_controller_axis_held(inputs, controller->c_right_trigger) > 0.5f;
      input.pickup |= fixed_input_controller_button_held(inputs, controller->c_r_bumper);
      input.drop |= fixed_input_controller_button_held(inputs, controller->c_l_bumper);
      input.sprint |= fixed_input_controller_axis_held(inputs, controller->c_left_trigger) > 0.5f;
    }

    const glm::vec2 r_nrm_dir = engine::normalize_safe({ input.rx, input.ry });
    input.rx = r_nrm_dir.x;
    input.ry = r_nrm_dir.y;

    // if (input.pickup)
    //   r.emplace_or_replace<WantsToPickUp>(entity);
    // if (input.drop)
    //   r.emplace_or_replace<WantsToDrop>(entity);
    // if (input.shoot)
    //   r.emplace_or_replace<WantsToShoot>(entity);
    // if (input.shoot_release)
    //   r.emplace_or_replace<WantsToReleaseShot>(entity);
    // if (input.sprint)
    //   r.emplace_or_replace<WantsToSprint>(entity);
    // if (input.sprint_release)
    //   r.emplace_or_replace<WantsToReleaseSprint>(entity);
  }

  update_movement_asteroids(r, milliseconds_dt);
  update_movement_direct(r);
  update_movement_jetpack(r);
};

} // namespace game2d
