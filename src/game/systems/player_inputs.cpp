#include "player_inputs.hpp"

#include "game/create_entities.hpp"
#include "modules/events/components.hpp"
#include "modules/networking/components.hpp"
#include "modules/physics/components.hpp"

#include <SDL2/SDL_mouse.h>

#include <iostream>
#include <vector>

void
game2d::update_player_inputs_system(entt::registry& r)
{
  const auto& inputs = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();
  if (inputs.history.size() == 0)
    return;
  const auto& unprocessed_update_input = inputs.history.back();

  const auto& view = r.view<const PlayerComponent, VelocityComponent>();
  view.each([&r, &unprocessed_update_input](auto entity, const auto& player, auto& vel) {
    for (int i = 0; i < unprocessed_update_input.size(); i++) {
      const auto& any_input = unprocessed_update_input[i];

      if (any_input.player != entity)
        continue; // wasn't this player's input

      switch (any_input.type) {
        case INPUT_TYPE::KEYBOARD: {
          if (any_input.key == player.W)
            vel.y = -1 * player.speed;
          if (any_input.key == player.S)
            vel.y = 1 * player.speed;
          if (any_input.key == player.A)
            vel.x = -1 * player.speed;
          if (any_input.key == player.D)
            vel.x = 1 * player.speed;

          if ((any_input.key == player.A || any_input.key == player.D) && any_input.release)
            vel.x = 0;
          if ((any_input.key == player.W || any_input.key == player.S) && any_input.release)
            vel.y = 0;
        }
        case INPUT_TYPE::MOUSE: {
          if (any_input.key == SDL_BUTTON_LEFT) {
            // Shoot()
            // entt::entity bullet = create_bullet(r);
            // const int BULLET_SPEED = 500;
            // const auto& mouse_pos = input.mouse_position_in_worldspace;
            // glm::vec2 dir = { mouse_pos.x - transform.position.x, mouse_pos.y - transform.position.y };
            // if (dir.x != 0.0f && dir.y != 0.0f)
            //   dir = glm::normalize(dir);
            // auto& bullet_velocity = r.get<VelocityComponent>(bullet);
            // bullet_velocity.x = dir.x * BULLET_SPEED;
            // bullet_velocity.y = dir.y * BULLET_SPEED;
            // auto& bullet_transform = r.get<TransformComponent>(bullet);
            // bullet_transform.position = transform.position;
            // float angle = engine::dir_to_angle_radians(dir);
            // bullet_transform.rotation.z = angle - engine::HALF_PI;
          }
        }
      }
    }
  });
}