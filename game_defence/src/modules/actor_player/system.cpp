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
#include <fmt/core.h>
#include <glm/glm.hpp>

namespace game2d {

void
update_movement_direct(entt::registry& r)
{
  const auto& view = r.view<const PlayerComponent, const InputComponent, const MovementDirectComponent, VelocityComponent>();
  for (const auto& [e, player_c, input_c, movetype_c, vel_c] : view.each()) {
    const glm::vec2 l_nrm_raw = { input_c.lx, input_c.ly };
    const glm::vec2 l_nrm_dir = engine::normalize_safe(l_nrm_raw);
    const glm::vec2 move_dir = (l_nrm_dir * vel_c.base_speed);

    vel_c.x = move_dir.x;
    vel_c.y = move_dir.y;
  }
};

void
update_movement_asteroids(entt::registry& r, const float dt)
{
  const auto& view =
    r.view<const PlayerComponent, const InputComponent, MovementAsteroidsComponent, VelocityComponent, TransformComponent>();
  for (const auto& [e, player_c, input_c, movetype_c, vel_c, transform_c] : view.each()) {

    const float rotation_speed = 2.0f;
    const float thrust_change = 100.0f;

    if (movetype_c.able_to_change_dir) {
      if (input_c.lx > 0) // rotate left
        transform_c.rotation_radians.z += dt * rotation_speed;
      if (input_c.lx < 0) // rotate right
        transform_c.rotation_radians.z -= dt * rotation_speed;
    }

    if (movetype_c.able_to_change_thrust) {
      if (input_c.ly > 0)
        movetype_c.thrust += dt * thrust_change;
      if (input_c.ly < 0)
        movetype_c.thrust -= dt * thrust_change;
      // hack: shift pressed, stop the ship
      if (input_c.sprint)
        movetype_c.thrust = 0;
    }

    const auto dir = engine::angle_radians_to_direction(transform_c.rotation_radians.z);
    vel_c.x = dir.x * movetype_c.thrust;
    vel_c.y = dir.y * movetype_c.thrust;
  }
}

void
update_player_controller_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const auto& finputs = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finputs.history.at(finputs.fixed_tick);

  const auto& view = r.view<const PlayerComponent, InputComponent, const AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, player, input, aabb] : view.each()) {

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
    if (input.shoot)
      r.emplace_or_replace<WantsToShoot>(entity);

    if (input.shoot_release)
      r.emplace_or_replace<WantsToReleaseShot>(entity);

    if (input.sprint)
      r.emplace_or_replace<WantsToSprint>(entity);

    if (input.sprint_release)
      r.emplace_or_replace<WantsToReleaseSprint>(entity);
  }

  update_movement_direct(r);
  update_movement_asteroids(r, milliseconds_dt / 1000.0f);
};

} // namespace game2d
