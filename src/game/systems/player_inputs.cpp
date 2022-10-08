#include "player_inputs.hpp"

#include "game/entities/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

#include "engine/maths/maths.hpp"

#include <imgui.h>

// maybe the player queue should have a
// list of all inputs pressed by a player

void
game2d::update_player_inputs_system(Game& g)
{
  const auto& p = g.physics;
  const auto& gameover = g.gameover;
  auto& input = g.input;
  auto& r = g.state;

  //
  // Capture player(s) inputs
  //

  const auto& view = r.view<PlayerComponent>();
  view.each([&input, &r](auto entity, auto& player) {
    // just capture inputs and process them in fixedupdate()

    for (const SDL_Event& evt : input.sdl_events) {
      if (evt.type == SDL_KEYDOWN && get_key_down(input, evt.key.keysym.scancode)) {
        InputEvent ie{ InputType::keyboard, entity, false, static_cast<uint32_t>(evt.key.keysym.scancode) };
        input.unprocessed_update_inputs.push_back(ie);
      }
      if (evt.type == SDL_KEYUP && get_key_up(input, evt.key.keysym.scancode)) {
        InputEvent ie{ InputType::keyboard, entity, true, static_cast<uint32_t>(evt.key.keysym.scancode) };
        input.unprocessed_update_inputs.push_back(ie);
      }
    }
    if (get_mouse_lmb_press())
      input.unprocessed_update_inputs.push_back({ InputType::mouse, entity, false, SDL_BUTTON_LEFT });
    if (get_mouse_lmb_release())
      input.unprocessed_update_inputs.push_back({ InputType::mouse, entity, true, SDL_BUTTON_LEFT });
    if (get_mouse_rmb_press())
      input.unprocessed_update_inputs.push_back({ InputType::mouse, entity, false, SDL_BUTTON_RIGHT });
    if (get_mouse_rmb_release())
      input.unprocessed_update_inputs.push_back({ InputType::mouse, entity, true, SDL_BUTTON_RIGHT });

    // .. rotate to velocity
    // Could improve this by not immediately setting rotation.z
    // and by setting a target angle that is lerped towards over multiple frames
    // const float EPSILON = 0.005f;
    // if (glm::abs(vel.x) > EPSILON || glm::abs(vel.y) > EPSILON)
    //   transform.rotation.z = engine::dir_to_angle_radians({ vel.x, vel.y }) - engine::HALF_PI;
  });
};