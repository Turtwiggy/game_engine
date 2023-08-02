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

    // hack: should used the fixed-input inputs
    if (controller && controller->controller != nullptr) {
      SDL_GameController* c = controller->controller;
      dx = get_axis_01(c, SDL_CONTROLLER_AXIS_LEFTX);
      dy = get_axis_01(c, SDL_CONTROLLER_AXIS_LEFTY);
      lmb_held = get_button_held(c, SDL_CONTROLLER_BUTTON_A);
    };

    // do the move
    const float speed = 1000.0f;

    const b2Vec2 dir = { dx * speed, dy * speed };
    actor.body->SetLinearVelocity(dir);
  }
};
