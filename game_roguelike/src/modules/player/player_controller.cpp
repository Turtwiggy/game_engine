#include "player_controller.hpp"

#include "components.hpp"
#include "maths/maths.hpp"
#include "helpers/check_equipment.hpp"
#include "modules/ai/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/player/components.hpp"
#include "modules/player/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/mouse.hpp"
#include "physics/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

void
game2d::update_player_controller_system(GameEditor& editor,
                                        Game& game,
                                        const std::vector<InputEvent>& inputs,
                                        const uint64_t& milliseconds_dt)
{
  auto& r = game.state;
  const auto& colours = editor.colours;

  // static bool ctrl_held = std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e){
  //   return e.type == InputType::keyboard && e.key == SDL_SCANCODE_LCTRL && e.state == InputState::held;
  // } ) != inputs.end();
  // std::cout << "ctrl_held" << ctrl_held << "\n";

  //
  // player movement
  //
  const auto& view = r.view<PlayerComponent, TransformComponent, GridMoveComponent>();
  for (auto [entity, player, transform, grid_move] : view.each()) {
    int dx = 0;
    int dy = 0;

    for (const InputEvent& input : inputs) {
      switch (input.type) {
        case InputType::keyboard: {
          if (input.key == player.W && (input.state == InputState::held || input.state == InputState::press))
            dy = -1;
          if (input.key == player.S && (input.state == InputState::held || input.state == InputState::press))
            dy = 1;
          if (input.key == player.A && (input.state == InputState::held || input.state == InputState::press))
            dx = -1;
          if (input.key == player.D && (input.state == InputState::held || input.state == InputState::press))
            dx = 1;

          // let keyboard overwrite path
          FollowPathComponent* potential_path = r.try_get<FollowPathComponent>(entity);
          if ((dx != 0 || dy) != 0 && potential_path)
            r.remove<FollowPathComponent>(entity);

          break;
        }

        case InputType::mouse: {
          if (input.key == SDL_BUTTON_LEFT && input.state == InputState::press) {
            entt::entity l_equipped = has_equipped(game, entity, EquipmentSlot::left_hand);
            if (l_equipped != entt::null && !input.hovering_over_ui)
              use_item(editor, game, entity, l_equipped);
          }
          if (input.key == SDL_BUTTON_RIGHT && input.state == InputState::press) {
            entt::entity r_equipped = has_equipped(game, entity, EquipmentSlot::right_hand);
            if (r_equipped != entt::null && !input.hovering_over_ui)
              use_item(editor, game, entity, r_equipped);
          }
          break;
        }

        case InputType::controller: {
          break; // nada
        }
      }

    } // end input each()

    // player.able_to_move = player.milliseconds_move_cooldown <= 0;
    // if (!player.able_to_move)
    //   player.milliseconds_move_cooldown -= milliseconds_dt;
    // if (player.able_to_move)
    //   player.milliseconds_move_cooldown = k_milliseconds_move_cooldown;

    // check if the entity is following a path
    if (auto* path = r.try_get<FollowPathComponent>(entity)) {
      if (!path)
        continue;
      path->k_milliseconds_between_path_updates_left -= milliseconds_dt;
      bool do_action = false;
      if (path->k_milliseconds_between_path_updates_left <= 0) {
        path->k_milliseconds_between_path_updates_left += k_milliseconds_between_ai_updates;
        do_action = true;
      }
      if (!do_action)
        continue;
      auto [dx, dy] = next_dir_along_path(path->calculated_path);
      if (path->calculated_path.size() > 0)
        path->calculated_path.erase(path->calculated_path.begin());
    }

    // do the move
    grid_move.x += 16 * dx;
    grid_move.y += 16 * dy;

    // gameplay: update player's line position
    // note: this probably doesn't belong here
    // const glm::ivec2& pos_player = transform.position;
    // const glm::ivec2 pos_mouse = mouse_position;
    // set_line(r, player.aim_line, pos_player, pos_mouse);

  } // end player each()
}
