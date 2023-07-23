#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "modules/player/components.hpp"
#include "modules/turret/helpers.hpp"
#include "physics/components.hpp"
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
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  // player movement
  const auto& view = r.view<const PlayerComponent, const TransformComponent, GridMoveComponent, InputComponent>();
  for (auto [entity, player, transform, grid, input] : view.each()) {

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

    // is there a turret near-by?
    const auto& info = get_closest(r, entity, transform, physics, EntityType::actor_turret);
    if (info.e != entt::null) {
      // draw a line from player to turret
      const auto a = glm::ivec2{ transform.position.x, transform.position.y };
      const auto& turret_transform = r.get<TransformComponent>(info.e);
      const auto b = glm::ivec2{ turret_transform.position.x, turret_transform.position.y };
      auto& line_transform = r.get<TransformComponent>(player.line);
      set_line(r, line_transform, a, b);
    }
  }
};
