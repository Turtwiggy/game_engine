#include "player_inputs.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

#include "engine/maths/maths.hpp"

#include <imgui.h>

// maybe the player queue should have a
// list of all inputs pressed by a player

void
game2d::update_player_inputs_system(entt::registry& r)
{
  const auto& p = r.ctx().at<SINGLETON_PhysicsComponent>();
  auto& gameover = r.ctx().at<SINGLETON_GameOverComponent>();
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();
  auto& input = r.ctx().at<SINGLETON_InputComponent>();

  //
  // Capture player(s) inputs
  //

  const auto& view = r.view<PlayerComponent>();
  view.each([&input, &r](auto entity, auto& player) {
    // just capture inputs and process them in fixedupdate()

    if (get_key_down(input, player.W))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.W) });
    if (get_key_down(input, player.A))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.A) });
    if (get_key_down(input, player.S))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.S) });
    if (get_key_down(input, player.D))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, false, static_cast<uint32_t>(player.D) });
    if (get_key_up(input, player.W))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.W) });
    if (get_key_up(input, player.A))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.A) });
    if (get_key_up(input, player.S))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.S) });
    if (get_key_up(input, player.D))
      input.unprocessed_update_inputs.push_back(
        { INPUT_TYPE::KEYBOARD, entity, true, static_cast<uint32_t>(player.D) });

    if (get_mouse_lmb_press())
      input.unprocessed_update_inputs.push_back({ INPUT_TYPE::MOUSE, entity, false, SDL_BUTTON_LEFT });
    if (get_mouse_lmb_release())
      input.unprocessed_update_inputs.push_back({ INPUT_TYPE::MOUSE, entity, true, SDL_BUTTON_LEFT });

    // .. rotate to velocity
    // Could improve this by not immediately setting rotation.z
    // and by setting a target angle that is lerped towards over multiple frames
    // const float EPSILON = 0.005f;
    // if (glm::abs(vel.x) > EPSILON || glm::abs(vel.y) > EPSILON)
    //   transform.rotation.z = engine::dir_to_angle_radians({ vel.x, vel.y }) - engine::HALF_PI;
  });
};