#include "player.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/networking/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/ui_editor_tilemap/components.hpp"

#include <SDL2/SDL_mouse.h>

#include <iostream>
#include <vector>

void
game2d::update_player_system(entt::registry& r, const std::vector<InputEvent>& inputs)
{
  auto& tilemap = game2d::get_first<TilemapComponent>(r);

  const auto& view = r.view<PlayerComponent, VelocityComponent>();
  view.each([&r, &inputs](entt::entity entity, PlayerComponent& player, VelocityComponent& vel) {
    for (int i = 0; i < inputs.size(); i++) {
      const auto& any_input = inputs[i];

      if (any_input.player != entity)
        continue; // wasn't this player's input

      switch (any_input.type) {
        case INPUT_TYPE::KEYBOARD: {

          if (any_input.key == player.W) {
            // vel.y = -1 * player.speed;
            vel.y = -16;
          }
          if (any_input.key == player.S) {
            // vel.y = 1 * player.speed;
            vel.y = 16;
          }
          if (any_input.key == player.A) {
            // vel.x = -1 * player.speed;
            vel.x = -16;
          }
          if (any_input.key == player.D) {
            // vel.x = 1 * player.speed;
            vel.x = 16;
          }
          if ((any_input.key == player.A || any_input.key == player.D) && any_input.release) {
            // vel.x = 0;
          }
          if ((any_input.key == player.W || any_input.key == player.S) && any_input.release) {
            // vel.y = 0;
          }
        }
        case INPUT_TYPE::MOUSE: {
          if (any_input.key == SDL_BUTTON_LEFT && !any_input.release) {

            // auto& equipment = player.hand_l;
            // if (equipment != nullptr) {
            //   std::vector<entt::entity> entities = { entity };
            //   // use eqipment
            //   if (equipment->use(r, entities)) {
            //     // destroy equpment?
            //     if (equipment->destroy_after_use) {
            //       if (equipment->count > 1) {
            //         equipment->count -= 1;
            //       } else {
            //         player.hand_l.reset();
            //         std::cout << "resetting" << "\n";
            //       }
            //     }
            //   }
            // }
          }
        }
      }
    }
  });
}