#include "system.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/physics/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

void
game2d::update_player_controller_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const auto& finputs = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finputs.history.at(finputs.fixed_tick);
  const auto& i = get_first_component<SINGLETON_InputComponent>(r);
  const auto dt = milliseconds_dt / 1000.0f;

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

  // todo: move these values in to player component
  const float player_speed = 50.0f;
  const float bullet_speed = 20.0f;
  const float gun_offset_distance = 30.0f;

  // player movement
  const auto& view = r.view<PlayerComponent, const AABB>();
  for (auto [entity, player, aabb] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    float lx = 0;
    float ly = 0;
    float rx = 0;
    float ry = 0;

    // actions
    bool shoot = false;
    bool pickup = false;
    bool sprint = false;
    bool place_turret = false;
    // bool reload = false;

    if (keyboard) {
      // todo: rx for keyboard
      ly += fixed_input_keyboard_held(SDL_SCANCODE_W) ? -1 : ly;
      ly += fixed_input_keyboard_held(SDL_SCANCODE_S) ? 1 : ly;
      lx += fixed_input_keyboard_held(SDL_SCANCODE_A) ? -1 : lx;
      lx += fixed_input_keyboard_held(SDL_SCANCODE_D) ? 1 : lx;
      shoot |= fixed_input_mouse_press(SDL_BUTTON_LEFT);
      pickup |= fixed_input_keyboard_press(SDL_SCANCODE_E);
      sprint |= fixed_input_keyboard_press(SDL_SCANCODE_LSHIFT);
      place_turret |= fixed_input_keyboard_press(SDL_SCANCODE_SPACE);
      // reload |= fixed_input_keyboard_press(SDL_SCANCODE_R);
    }

    if (controller) {
      lx += fixed_input_controller_axis_held(controller->c_left_stick_x);
      ly += fixed_input_controller_axis_held(controller->c_left_stick_y);
      rx += fixed_input_controller_axis_held(controller->c_right_stick_x);
      ry += fixed_input_controller_axis_held(controller->c_right_stick_y);
      shoot |= fixed_input_controller_axis_held(controller->c_right_trigger) != 0.0f;
      pickup |= fixed_input_controller_button_held(controller->c_r_bumper);
      sprint |= fixed_input_controller_button_held(controller->c_l_bumper);
      place_turret |= fixed_input_controller_button_press(controller->c_y);
      // reload |=
    }

    glm::vec2 r_nrm_dir = { rx, ry };
    if (r_nrm_dir.x != 0.0f || r_nrm_dir.y != 0.0f)
      r_nrm_dir = glm::normalize(r_nrm_dir);

    glm::vec2 l_nrm_dir = { lx, ly };
    if (l_nrm_dir.x != 0.0f || l_nrm_dir.y != 0.0f)
      l_nrm_dir = glm::normalize(l_nrm_dir);

    // do the move
    if (auto* vel = r.try_get<VelocityComponent>(entity)) {
      const glm::vec2 move_dir = l_nrm_dir * player_speed;
      vel->x = move_dir.x;
      vel->y = move_dir.y;

      if (sprint) {
        vel->x *= 2.0f;
        vel->y *= 2.0f;
      }
    }

    // offset the bullet by a distance
    // to stop the bullet spawning inside the entity
    glm::ivec2 offset_pos = { r_nrm_dir.x * gun_offset_distance, r_nrm_dir.y * gun_offset_distance };
    player.offset = offset_pos;

    // visually display the gun angle
    // note: this should be in update() not fixedupdate()
    if (glm::abs(r_nrm_dir.x) + glm::abs(r_nrm_dir.y) > 0.001f) {
      auto& debug_gunspot_transform = r.emplace_or_replace<TransformComponent>(player.debug_gun_spot);
      debug_gunspot_transform.position.x = aabb.center.x + offset_pos.x;
      debug_gunspot_transform.position.y = aabb.center.y + offset_pos.y;
      debug_gunspot_transform.scale.x = 5.0f;
      debug_gunspot_transform.scale.y = 5.0f;

    } else {
      if (auto* gun_transform = r.try_get<TransformComponent>(player.debug_gun_spot))
        r.remove<TransformComponent>(player.debug_gun_spot);
    }

    //
    // request to shoot
    //
    {
      bool allowed_to_shoot = true;

      // hack: reload
      // if (player.time_between_reloads_left > 0.0f)
      //   player.time_between_reloads_left -= dt;
      // if (reload && player.time_between_reloads_left <= 0.0f) {
      //   player.bullets_in_clip_left = player.bullets_in_clip;
      //   player.time_between_reloads_left = player.time_between_reloads;
      // }
      // allowed_to_shoot &= player.bullets_in_clip_left > 0;          // enough bullets
      // if(shoot && allowed_to_shoot)
      //   player.bullets_in_clip_left--;

      // bullet cooldown
      if (player.time_between_bullets_left > 0.0f)
        player.time_between_bullets_left -= dt;
      allowed_to_shoot &= player.time_between_bullets_left <= 0.0f;

      if (shoot && allowed_to_shoot) {

        CreateEntityRequest req;
        req.type = EntityType::actor_bullet;
        req.position = { aabb.center.x + offset_pos.x, aabb.center.y + offset_pos.y, 0 };
        req.velocity = glm::vec3(r_nrm_dir.x * bullet_speed, r_nrm_dir.y * bullet_speed, 0);
        r.emplace<CreateEntityRequest>(r.create(), req);

        // request audio
        // r.emplace<AudioRequestPlayEvent>(r.create(), "SHOOT_01");

        // reset timer
        player.time_between_bullets_left = player.time_between_bullets;
      }
    }

    if (pickup) {
      std::cout << "Pickup pressed!\n";
      r.emplace_or_replace<WantsToPickUp>(entity);
    }

    //
    // hack: place turret?
    // bug: turrets_placed not reset on scene reset
    //
    if (place_turret && player.turrets_placed < 4) {
      player.turrets_placed++;
      CreateEntityRequest req;
      req.type = EntityType::actor_turret;
      req.position = { aabb.center.x, aabb.center.y, 0 };
      r.emplace<CreateEntityRequest>(r.create(), req);
    }
  }
};
