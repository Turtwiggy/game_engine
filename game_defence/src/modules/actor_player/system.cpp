#include "system.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "sprites/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

void
game2d::update_player_controller_system(entt::registry& r)
{
  const auto& finputs = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finputs.history.at(finputs.fixed_tick);
  const auto& i = get_first_component<SINGLETON_InputComponent>(r);

  const auto fixed_input_keyboard_press = [&inputs](const SDL_Scancode& key) -> bool {
    const auto l = [&key](const InputEvent& e) {
      return (e.type == InputType::keyboard && e.keyboard == key && e.state == InputState::press);
    };
    return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
  };

  const auto fixed_input_keyboard_held = [&inputs](const SDL_Scancode& key) -> bool {
    const auto l = [&key](const InputEvent& e) {
      return (e.type == InputType::keyboard && e.keyboard == key && e.state == InputState::held);
    };
    return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
  };

  // SDL_BUTTON_LEFT = 1; SDL_BUTTON_MIDDLE = 2; SDL_BUTTON_RIGHT = 3;
  const auto fixed_input_mouse_press = [&inputs](const int& button) -> bool {
    const auto& l = [&button](const InputEvent& e) {
      return (e.type == InputType::mouse && e.mouse == button && e.state == InputState::press);
    };
    return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
  };

  const auto fixed_input_controller_button_press = [&inputs](const SDL_GameControllerButton& button) -> bool {
    const auto& l = [&button](const InputEvent& e) {
      return (e.type == InputType::controller && e.state == InputState::press &&
              e.joystick_event == JoystickEventType::button &&
              // e.joystick_id ==  // any id
              e.controller_button == button);
    };
    return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
  };

  const auto fixed_input_controller_button_held = [&inputs](const SDL_GameControllerButton& button) -> bool {
    const auto& l = [&button](const InputEvent& e) {
      return (e.type == InputType::controller && e.state == InputState::held &&
              e.joystick_event == JoystickEventType::button && e.controller_button == button);
    };
    return std::find_if(inputs.begin(), inputs.end(), l) != std::end(inputs);
  };

  const auto fixed_input_controller_axis_held = [&inputs](const SDL_GameControllerAxis& axis) -> float {
    // process more recent inputs first
    for (int i = inputs.size() - 1; i >= 0; i--) {
      const auto& e = inputs[i];
      const bool is_controller = e.type == InputType::controller;
      const bool is_axis = e.joystick_event == JoystickEventType::axis;
      const bool your_axis = e.controller_axis == axis;
      if (is_controller && is_axis && your_axis)
        return e.controller_axis_value_01;
    }
    return 0.0f;
  };

  // player movement
  const auto& view = r.view<PlayerComponent, InputComponent, const AABB>();
  for (const auto& [entity, player, input, aabb] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    input.rx = 0.0f;
    input.ry = 0.0f;
    input.lx = 0.0f;
    input.ly = 0.0f;
    input.shoot = false;
    input.pickup = false;
    input.sprint = false;
    input.place_turret = false;

    if (keyboard) {
      // todo: rx for keyboard
      input.ly += fixed_input_keyboard_held(SDL_SCANCODE_W) ? -1 : 0;
      input.ly += fixed_input_keyboard_held(SDL_SCANCODE_S) ? 1 : 0;
      input.lx += fixed_input_keyboard_held(SDL_SCANCODE_A) ? -1 : 0;
      input.lx += fixed_input_keyboard_held(SDL_SCANCODE_D) ? 1 : 0;
      input.shoot |= fixed_input_mouse_press(SDL_BUTTON_LEFT);
      input.pickup |= fixed_input_keyboard_press(SDL_SCANCODE_E);
      input.sprint |= fixed_input_keyboard_press(SDL_SCANCODE_LSHIFT);
      input.place_turret |= fixed_input_keyboard_press(SDL_SCANCODE_SPACE);
      // reload |= fixed_input_keyboard_press(SDL_SCANCODE_R);
    }

    if (controller) {
      input.lx += fixed_input_controller_axis_held(controller->c_left_stick_x);
      input.ly += fixed_input_controller_axis_held(controller->c_left_stick_y);
      input.rx += fixed_input_controller_axis_held(controller->c_right_stick_x);
      input.ry += fixed_input_controller_axis_held(controller->c_right_stick_y);
      input.shoot |= fixed_input_controller_axis_held(controller->c_right_trigger) > 0.5f;
      input.pickup |= fixed_input_controller_button_held(controller->c_r_bumper);
      input.sprint |= fixed_input_controller_button_held(controller->c_l_bumper);
      input.place_turret |= fixed_input_controller_button_press(controller->c_y);
      // reload |=
    }

    //
    // do the move
    //
    if (auto* vel = r.try_get<VelocityComponent>(entity)) {

      glm::vec2 l_nrm_dir = { input.lx, input.ly };
      if (l_nrm_dir.x != 0.0f || l_nrm_dir.y != 0.0f)
        l_nrm_dir = glm::normalize(l_nrm_dir);

      const glm::vec2 move_dir = l_nrm_dir * vel->base_speed;
      vel->x = move_dir.x;
      vel->y = move_dir.y;

      if (input.sprint) {
        vel->x *= 2.0f;
        vel->y *= 2.0f;
      }
    }

    if (input.pickup) {
      std::cout << "Pickup pressed!\n";
      r.emplace_or_replace<WantsToPickUp>(entity);
    }

    //
    // hack: give player a weapon on spawn
    //
    if (!player.has_weapon) {
      player.has_weapon = true;

      CreateEntityRequest req;
      req.type = player.weapon_to_spawn_with;
      req.parent = entity;
      r.emplace<CreateEntityRequest>(r.create(), req);
    }
  }
};
