#include "modules/actor_player/system.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/fixed_update.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "maths/maths.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
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
  const float dt = milliseconds_dt / 1000.0f;

  // const auto cursor = get_first<CursorComponent>(r);
  // const auto& cursor_transform = r.get<TransformComponent>(cursor);

  // player movement
  const auto& view =
    r.view<PlayerComponent, InputComponent, const AABB, const TransformComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, player, input, aabb, transform] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    // convert input to actions
    //
    {
      input.rx = 0.0f;
      input.ry = 0.0f;
      input.lx = 0.0f;
      input.ly = 0.0f;
      input.shoot = false;
      input.shoot_release = false;
      input.pickup = false;
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
        input.shoot_release |= fixed_input_mouse_release(inputs, SDL_BUTTON_LEFT);
        input.pickup |= fixed_input_keyboard_press(inputs, keyboard->pickup);
        input.sprint |= fixed_input_keyboard_press(inputs, keyboard->sprint);
        // input.place_turret |= fixed_input_keyboard_press(SDL_SCANCODE_SPACE);
        // reload |= fixed_input_keyboard_press(SDL_SCANCODE_R);
      }

      if (controller) {
        input.lx += fixed_input_controller_axis_held(inputs, controller->c_left_stick_x);
        input.ly += fixed_input_controller_axis_held(inputs, controller->c_left_stick_y);
        input.rx += fixed_input_controller_axis_held(inputs, controller->c_right_stick_x);
        input.ry += fixed_input_controller_axis_held(inputs, controller->c_right_stick_y);
        input.shoot |= fixed_input_controller_axis_held(inputs, controller->c_right_trigger) > 0.5f;
        // input.shoot_release |= fixed_input_controller_axis_held(inputs, controller->c_right_trigger) <= 0.1f;
        input.pickup |= fixed_input_controller_button_held(inputs, controller->c_r_bumper);
        input.sprint |= fixed_input_controller_button_held(inputs, controller->c_l_bumper);
        // input.place_turret |= fixed_input_controller_button_press(controller->c_y);
        // reload |=
      }

      const glm::vec2 r_nrm_dir = engine::normalize_safe({ input.rx, input.ry });
      input.rx = r_nrm_dir.x;
      input.ry = r_nrm_dir.y;
    }

    if (input.pickup)
      std::cout << "Pickup pressed!" << std::endl;

    // shoot action
    //
    if (input.shoot)
      r.emplace_or_replace<WantsToShoot>(entity);
    if (input.shoot_release)
      r.emplace_or_replace<WantsToReleaseShot>(entity);

    // do the move..
    //
    //

    // if (auto* vel = r.try_get<VelocityComponent>(entity)) {
    //   const glm::vec2 l_nrm_raw = { input.lx, input.ly };
    //   const glm::vec2 l_nrm_dir = engine::normalize_safe(l_nrm_raw);
    //   const glm::vec2 move_dir = (l_nrm_dir * vel->base_speed) / dt;
    //   vel->x = move_dir.x;
    //   vel->y = move_dir.y;
    //   if (input.sprint) {
    //     vel->x *= 2.0f;
    //     vel->y *= 2.0f;
    //   }
    // }
  }
};
