#include "player_controller.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "game/helpers/line.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
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
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);

  const auto& view = r.view<PlayerComponent, TransformComponent, GridMoveComponent>();
  view.each([&r, &inputs, &mouse_position](
              entt::entity entity, PlayerComponent& player, TransformComponent& transform, GridMoveComponent& grid) {
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
            //
          };

          if (any_input.key == SDL_BUTTON_RIGHT) {
            if (!any_input.release)
              r.emplace_or_replace<TransformComponent>(player.aim_line);
            else
              r.remove<TransformComponent>(player.aim_line);
          }
        }
      }
    }

    // update player's line position
    // note: this probably doesn't belong here
    const glm::ivec2& pos_player = transform.position;
    const glm::ivec2 pos_mouse = mouse_position;
    set_line(r, player.aim_line, pos_player, pos_mouse);
  });
}