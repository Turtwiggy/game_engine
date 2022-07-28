#include "player_inputs.hpp"

#include "game/create_entities.hpp"
#include "modules/events/components.hpp"

#include <iostream>

void
game2d::update_player_inputs_system(entt::registry& r)
{
  auto& input = r.ctx().at<SINGLETON_InputComponent>();
  const auto& view = r.view<PlayerComponent>();

  view.each([&input, &r](auto& player) {
    // process all inputs

    int size = player.unprocessed_keyboard_inputs.size();
    if (size > 0)
      std::cout << "processing " << size << std::endl;

    // TODO: actually process the events
    // while (!player.unprocessed_keyboard_inputs.empty())
    //   player.unprocessed_keyboard_inputs.pop();
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