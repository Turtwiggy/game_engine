#include "player_controller.hpp"

#include "components.hpp"
#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
#include "game/modules/player/components.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/networking/components.hpp"
#include "modules/physics/components.hpp"

#include <SDL2/SDL_mouse.h>

void
game2d::update_player_controller_system(GameEditor& editor,
                                        Game& game,
                                        const std::vector<InputEvent>& inputs,
                                        const uint64_t& milliseconds_dt)
{
  auto& r = game.state;
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(editor, game);

  //
  // player movement
  //

  const auto& view = r.view<PlayerComponent, TransformComponent, GridMoveComponent>();
  view.each([&r, &inputs, &mouse_position, &editor, &game, &milliseconds_dt](
              entt::entity entity, PlayerComponent& player, TransformComponent& transform, GridMoveComponent& grid) {
    //

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
        }
        case InputType::mouse: {
          if (input.key == SDL_BUTTON_LEFT && input.state == InputState::press) {

            bool do_fire = false;
            if (!do_fire)
              break;

            const float bullet_speed = 50.0f;
            entt::entity bullet = create_gameplay(editor, r, EntityType::arrow);
            create_renderable(editor, r, bullet, EntityType::arrow);
            glm::vec2 dir = { mouse_position.x - transform.position.x, mouse_position.y - transform.position.y };
            if (dir.x != 0.0f && dir.y != 0.0f)
              dir = glm::normalize(dir);

            TransformComponent& bullet_transform = r.get<TransformComponent>(bullet);
            bullet_transform.position = transform.position;

            VelocityComponent& vel = r.get<VelocityComponent>(bullet);
            vel.x = dir.x * bullet_speed;
            vel.y = dir.y * bullet_speed;

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

          if (input.key == SDL_BUTTON_RIGHT) {
          };
        }
      }
    }

    // player.able_to_move = player.milliseconds_move_cooldown <= 0;
    // if (!player.able_to_move)
    //   player.milliseconds_move_cooldown -= milliseconds_dt;
    // if (player.able_to_move)
    //   player.milliseconds_move_cooldown = k_milliseconds_move_cooldown;

    // do the move
    grid.x += 16 * dx;
    grid.y += 16 * dy;

    // gameplay: update player's line position
    // note: this probably doesn't belong here
    // const glm::ivec2& pos_player = transform.position;
    // const glm::ivec2 pos_mouse = mouse_position;
    // set_line(r, player.aim_line, pos_player, pos_mouse);
  });
}
