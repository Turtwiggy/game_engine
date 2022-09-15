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
    for (const InputEvent& input : inputs) {
      //
      if (input.player != entity)
        continue; // wasn't this player's input

      switch (input.type) {
        case InputType::keyboard: {

          int dx = 0;
          int dy = 0;
          if (input.key == player.W && !input.release)
            dy = -1;
          if (input.key == player.S && !input.release)
            dy = 1;
          if (input.key == player.A && !input.release)
            dx = -1;
          if (input.key == player.D && !input.release)
            dx = 1;

          grid.x += 16 * dx;
          grid.y += 16 * dy;
        }
        case InputType::mouse: {
          if (input.key == SDL_BUTTON_LEFT && !input.release) {
            //
          };

          if (input.key == SDL_BUTTON_RIGHT) {
            if (!input.release)
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