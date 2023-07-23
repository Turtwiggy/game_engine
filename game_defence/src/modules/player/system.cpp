#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

#include "physics/components.hpp"

void
game2d::update_player_controller_system(entt::registry& r,
                                        const std::vector<InputEvent>& inputs,
                                        const uint64_t& milliseconds_dt)
{
  const auto& sdl_inputs = get_first_component<SINGLETON_InputComponent>(r);

  // bool ctrl_held = std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e) {
  //                    return e.type == InputType::keyboard && e.key == SDL_SCANCODE_LCTRL && e.state == InputState::held;
  //                  }) != std::end(inputs);
  // std::cout << "(FIXEDTICK) ctrl_held: " << ctrl_held << "\n";

  // player movement
  const auto& view = r.view<const PlayerComponent, GridMoveComponent, InputComponent>();
  for (auto [entity, player, grid, input] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    float& dx = input.dir.x;
    float& dy = input.dir.y;

    if (keyboard) {
      for (const InputEvent& i : inputs) {
        auto held = i.state == InputState::held;
        auto press = i.state == InputState::press;
        auto release = i.state == InputState::release;
        if (i.key == keyboard->W && (held || press))
          dy = -1;
        if (i.key == keyboard->S && (held || press))
          dy = 1;
        if (i.key == keyboard->A && (held || press))
          dx = -1;
        if (i.key == keyboard->D && (held || press))
          dx = 1;
        if ((i.key == keyboard->W || i.key == keyboard->S) && release)
          dy = 0;
        if ((i.key == keyboard->A || i.key == keyboard->D) && release)
          dx = 0;
      }
    }
    const auto& available_controllers = sdl_inputs.controllers;
    if (controller && available_controllers.size() > 0) {
      auto* c = available_controllers[0];
      dx = get_axis_01(c, SDL_CONTROLLER_AXIS_LEFTX);
      dy = get_axis_01(c, SDL_CONTROLLER_AXIS_LEFTY);
    }

    // do the move
    const float speed = 250.0f;
    grid.x += dx * speed * (milliseconds_dt / 1000.0f);
    grid.y += dy * speed * (milliseconds_dt / 1000.0f);
  }
};
