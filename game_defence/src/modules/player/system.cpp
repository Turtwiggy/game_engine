#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "modules/physics_box2d/components.hpp"
#include "modules/player/components.hpp"
#include "modules/turret/components.hpp"
#include "modules/turret/helpers.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "sprites/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

void
game2d::update_player_controller_system(entt::registry& r,
                                        const std::vector<InputEvent>& inputs,
                                        const uint64_t& milliseconds_dt)
{
  const auto& sdl_inputs = get_first_component<SINGLETON_InputComponent>(r);
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  // player movement
  const auto& view = r.view<const PlayerComponent, ActorComponent, const TransformComponent, InputComponent>();
  for (auto [entity, player, actor, transform, input] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    float& dx = input.dir.x;
    float& dy = input.dir.y;

    bool lmb_held = false;
    lmb_held |= get_mouse_lmb_held();

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
      lmb_held = get_button_held(c, SDL_CONTROLLER_BUTTON_A);
    }

    // do the move
    const float speed = 1000.0f;

    b2Vec2 dir = { dx * speed, dy * speed };
    // printf("applying force: %f %f", dir.x, dir.y);
    // actor.body->ApplyForceToCenter(dir, true);
    actor.body->SetLinearVelocity(dir);

    // grid.x += dx * speed * (milliseconds_dt / 1000.0f);
    // grid.y += dy * speed * (milliseconds_dt / 1000.0f);

    // is there a turret near-by?
    // const auto& info = get_closest(r, entity, transform, physics, EntityType::actor_turret);
    // if (info.e != entt::null) {
    //   // draw a line from player to turret
    //   const auto a = glm::ivec2{ transform.position.x, transform.position.y };
    //   const auto& turret_transform = r.get<TransformComponent>(info.e);
    //   const auto b = glm::ivec2{ turret_transform.position.x, turret_transform.position.y };
    //   auto& line_transform = r.get<TransformComponent>(player.line);
    //   set_line(r, line_transform, a, b);
    // }
  }
};
