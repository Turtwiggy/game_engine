#include "player_controller.hpp"

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
game2d::update_player_controller_system(entt::registry& r, const std::vector<InputEvent>& inputs)
{
  auto& tilemap = game2d::get_first<TilemapComponent>(r);

  const auto& view = r.view<PlayerComponent, GridMoveComponent>();
  view.each([&r, &inputs](entt::entity entity, PlayerComponent& player, GridMoveComponent& grid) {
    for (int i = 0; i < inputs.size(); i++) {
      const auto& any_input = inputs[i];

      if (any_input.player != entity)
        continue; // wasn't this player's input

      switch (any_input.type) {
        case INPUT_TYPE::KEYBOARD: {

          int dx = 0;
          int dy = 0;
          if (any_input.key == player.W && !any_input.release)
            dy = -1;
          if (any_input.key == player.S && !any_input.release)
            dy = 1;
          if (any_input.key == player.A && !any_input.release)
            dx = -1;
          if (any_input.key == player.D && !any_input.release)
            dx = 1;

          grid.x += 16 * dx;
          grid.y += 16 * dy;
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