#include "player_inputs.hpp"

#include "game/create_entities.hpp"
#include "modules/events/components.hpp"

#include <iostream>
#include <string>

void
game2d::update_player_inputs_system(entt::registry& r)
{
  {
    // move all unprocessed inputs from Update() to FixedUpdate()
    auto& input = r.ctx().at<SINGLETON_InputComponent>();
    auto& fixed_input = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();
    fixed_input.history.emplace(std::move(input.unprocessed_update_inputs));

    // only store the last X fixed_inputs
    if (fixed_input.history.size() > fixed_input.max_history_size)
      fixed_input.history.pop();
  }

  const auto& inputs = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();
  const auto& unprocessed_update_input = inputs.history.back();

  const auto& view = r.view<PlayerComponent>();
  view.each([&r, &unprocessed_update_input](auto entity, auto& player) {
    for (int i = 0; i < unprocessed_update_input.size(); i++) {
      const auto& any_input = unprocessed_update_input[i];
      if (any_input.player != entity)
        continue; // wasn't this player's input
      // was this player's input
      //
      // TODO: (this)
      std::cout << "Processing player input: " << std::to_string(any_input.key) << std::endl;
    }
  });

  // if (get_key_held(input, SDL_SCANCODE_W))
  //   vel.y = -1 * player.speed;
  // if (get_key_held(input, SDL_SCANCODE_S))
  //   vel.y = 1 * player.speed;
  // if (get_key_held(input, SDL_SCANCODE_A))
  //   vel.x = -1 * player.speed;
  // if (get_key_held(input, SDL_SCANCODE_D))
  //   vel.x = 1 * player.speed;
  // if (get_key_up(input, SDL_SCANCODE_A) || get_key_up(input, SDL_SCANCODE_D))
  //   vel.x = 0.0f;
  // if (get_key_up(input, SDL_SCANCODE_W) || get_key_up(input, SDL_SCANCODE_S))
  //   vel.y = 0.0f;

  // // remove all events!
  // while (!input.sdl_events.empty()) {
  //   AppEvent event = input.sdl_events.front();
  //   std::cout << "FixedUpdate() Processing event from Update(): " << event.evt.type << std::endl;
  //   if (event.evt.type == SDL_KEYDOWN)
  //     std::cout << "FixedUpdate() would process key down event..." << std::endl;
  //   if (event.evt.type == SDL_KEYUP)
  //     std::cout << "FixedUpdate() would process key up event..." << std::endl;
  //   input.sdl_events.pop();
  // }

  // iterate over all events
  // for (int i = 0; i < input.sdl_events.size(); i++) {
  //   // pop each element, push it to back because yolo
  //   AppEvent elem = std::move(input.sdl_events.front());
  //   input.sdl_events.pop();
  //   input.sdl_events.push(elem);
  //   if (elem.evt.type == SDL_KEYDOWN) {
  //     if (elem.evt.key.keysym.scancode == SDL_SCANCODE_RETURN) {
  //       std::cout << "doing fixed update..." << std::endl;
  //       do_fixed_update = true;
  //     }
  //   }
  // }

  // Shoot()
  //   entt::entity bullet = create_bullet(r);
  //   const int BULLET_SPEED = 500;
  //   const auto& mouse_pos = input.mouse_position_in_worldspace;
  //   glm::vec2 dir = { mouse_pos.x - transform.position.x, mouse_pos.y - transform.position.y };
  //   if (dir.x != 0.0f && dir.y != 0.0f)
  //     dir = glm::normalize(dir);
  //   auto& bullet_velocity = r.get<VelocityComponent>(bullet);
  //   bullet_velocity.x = dir.x * BULLET_SPEED;
  //   bullet_velocity.y = dir.y * BULLET_SPEED;
  //   auto& bullet_transform = r.get<TransformComponent>(bullet);
  //   bullet_transform.position = transform.position;
  //   float angle = engine::dir_to_angle_radians(dir);
  //   bullet_transform.rotation.z = angle - engine::HALF_PI;
}