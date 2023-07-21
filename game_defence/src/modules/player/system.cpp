#include "system.hpp"

#include "components.hpp"
#include "events/components.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

#include "physics/components.hpp"

void
game2d::update_player_controller_system(entt::registry& r,
                                        const std::vector<InputEvent>& inputs,
                                        const uint64_t& milliseconds_dt)
{
  // bool ctrl_held = std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e) {
  //                    return e.type == InputType::keyboard && e.key == SDL_SCANCODE_LCTRL && e.state == InputState::held;
  //                  }) != std::end(inputs);
  // std::cout << "(FIXEDTICK) ctrl_held: " << ctrl_held << "\n";

  // player movement
  const auto& view = r.view<const PlayerComponent, GridMoveComponent, InputComponent>();
  for (auto [entity, player, grid, input] : view.each()) {

    int& dx = input.dir.x;
    int& dy = input.dir.y;

    for (const InputEvent& i : inputs) {
      switch (i.type) {
        case InputType::keyboard: {
          auto held = i.state == InputState::held;
          auto press = i.state == InputState::press;
          auto release = i.state == InputState::release;

          if (i.key == player.W && (held || press))
            dy = -1;
          if (i.key == player.S && (held || press))
            dy = 1;
          if (i.key == player.A && (held || press))
            dx = -1;
          if (i.key == player.D && (held || press))
            dx = 1;

          if ((i.key == player.W || i.key == player.S) && release)
            dy = 0;
          if ((i.key == player.A || i.key == player.D) && release)
            dx = 0;

          break;
        }

        case InputType::mouse: {
          // if (i.key == SDL_BUTTON_LEFT && i.state == InputState::press) {
          //   entt::entity l_equipped = has_equipped(game, entity, EquipmentSlot::left_hand);
          //   if (l_equipped != entt::null && !input.hovering_over_ui)
          //     use_item(editor, game, entity, l_equipped);
          // }
          // if (i.key == SDL_BUTTON_RIGHT && i.state == InputState::press) {
          //   entt::entity r_equipped = has_equipped(game, entity, EquipmentSlot::right_hand);
          //   if (r_equipped != entt::null && !input.hovering_over_ui)
          //     use_item(editor, game, entity, r_equipped);
          // }
          break;
        }

        case InputType::controller: {
          break; // nada
        }
      }
    } // end input each()

    // do the move
    float speed = 1000.0f;
    grid.x += static_cast<int>(dx * speed * (milliseconds_dt / 1000.0f));
    grid.y += static_cast<int>(dy * speed * (milliseconds_dt / 1000.0f));

  } // end player each()
}
