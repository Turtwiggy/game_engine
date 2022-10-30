#include "player_controller.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/ai/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"

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
  const int GRID_SIZE = 16;
  const auto offset = glm::ivec2(GRID_SIZE / 2, GRID_SIZE / 2);
  const auto mouse_position = mouse_position_in_worldspace(editor, game) + offset;
  const auto mouse_grid = engine::grid::world_space_to_grid_space(mouse_position, GRID_SIZE);
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const auto& group = r.group<GridComponent, PathfindableComponent>();

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

          FollowPathComponent* potential_path = r.try_get<FollowPathComponent>(entity);
          // let keyboard overwrite path
          if ((dx != 0 || dy) != 0 && potential_path)
            r.remove<FollowPathComponent>(entity);

          break;
        }
        case InputType::mouse: {
          if (input.key == SDL_BUTTON_LEFT && input.state == InputState::press) {

            // player position
            const auto player_grid =
              engine::grid::world_space_to_grid_space({ transform.position.x, transform.position.y }, GRID_SIZE);
            vec2i from = { glm::clamp(player_grid.x, 0, d.width - 1), glm::clamp(player_grid.y, 0, d.height - 1) };

            // mouse position
            vec2i to = { glm::clamp(mouse_grid.x, 0, d.width - 1), glm::clamp(mouse_grid.y, 0, d.height - 1) };
            std::cout << "pathfinding player to..." << to.x << " " << to.y << "\n";

            // pathfind to location
            const auto path = astar(r, from, to);

            // Set new destination
            FollowPathComponent* potential_path = r.try_get<FollowPathComponent>(entity);
            if (potential_path) {
              potential_path->calculated_path.clear();
              potential_path->calculated_path = path;
            } else {
              FollowPathComponent& new_path = r.emplace<FollowPathComponent>(entity);
              new_path.calculated_path = path;
            }

            // bool do_fire = false;
            // if (!do_fire)
            //   break;
            // const float bullet_speed = 50.0f;
            // entt::entity bullet = create_gameplay(editor, r, EntityType::arrow);
            // create_renderable(editor, r, bullet, EntityType::arrow);
            // glm::vec2 dir = { mouse_position.x - transform.position.x, mouse_position.y - transform.position.y };
            // if (dir.x != 0.0f && dir.y != 0.0f)
            //   dir = glm::normalize(dir);
            // TransformComponent& bullet_transform = r.get<TransformComponent>(bullet);
            // bullet_transform.position = transform.position;
            // VelocityComponent& vel = r.get<VelocityComponent>(bullet);
            // vel.x = dir.x * bullet_speed;
            // vel.y = dir.y * bullet_speed;

            //  bullet_transform.rotation.z = angle - engine::HALF_PI;
            // // add some randomness to the bullet's direction
            // // todo: replace complete randomness with a recoil factor.
            // constexpr float angle_min_max = 10.0f * engine::PI / 180.0f;
            // const float random_angle = engine::rand_det_s(rnd.rng, -angle_min_max, angle_min_max);
            // Create an attack ID
            // std::cout << "bullet attack, attack id: " << a.id << "\n";
            // Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
            // gs.attacks.push_back(a);
          };

          break;
        }
        case InputType::controller: {
          // nada
          break;
        }
      }
    }

    // player.able_to_move = player.milliseconds_move_cooldown <= 0;
    // if (!player.able_to_move)
    //   player.milliseconds_move_cooldown -= milliseconds_dt;
    // if (player.able_to_move)
    //   player.milliseconds_move_cooldown = k_milliseconds_move_cooldown;

    // do the move
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

      if (path->calculated_path.size() > 0) {
        const auto& from = path->calculated_path[0]; // path[0] is current
        const auto& to = path->calculated_path[1];
        int dx = to.x - from.x;
        int dy = to.y - from.y;
        grid_move.x += 16 * dx;
        grid_move.y += 16 * dy;
        path->calculated_path.erase(path->calculated_path.begin());
      }

    } else {
      grid_move.x += 16 * dx;
      grid_move.y += 16 * dy;
    }

    // gameplay: update player's line position
    // note: this probably doesn't belong here
    // const glm::ivec2& pos_player = transform.position;
    // const glm::ivec2 pos_mouse = mouse_position;
    // set_line(r, player.aim_line, pos_player, pos_mouse);
  };
}
