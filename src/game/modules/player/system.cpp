#include "system.hpp"

#include "components.hpp"

#include "engine/maths/maths.hpp"
#include "game/components/actors.hpp"
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
game2d::update_player_controller_system(GameEditor& editor, Game& game, const std::vector<InputEvent>& inputs)
{
  auto& r = game.state;
  auto& tilemap = game2d::get_first<TilemapComponent>(r);
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(editor, game);

  const auto& view = r.view<PlayerComponent, TransformComponent, GridMoveComponent>();
  view.each([&r, &inputs, &mouse_position, &editor, &game](
              entt::entity entity, PlayerComponent& player, TransformComponent& transform, GridMoveComponent& grid) {
    for (const InputEvent& input : inputs) {
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

            // shoot()

            const float bullet_speed = 50.0f;

            entt::entity bullet = create_gameplay(editor, game, EntityType::arrow);
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

    // gameplay: update player's line position
    // note: this probably doesn't belong here
    // const glm::ivec2& pos_player = transform.position;
    // const glm::ivec2 pos_mouse = mouse_position;
    // set_line(r, player.aim_line, pos_player, pos_mouse);
  });
}

// bool
// Sword::use(entt::registry& r, std::vector<entt::entity>& entities)
// {
//   std::cout << "slashin..!" << "\n";
// s.slash_attack_time_left = s.slash_attack_time;
// s.attack_left_to_right = !s.attack_left_to_right; // keep swapping left to right to right to left etc
// if (s.attack_left_to_right)
//   s.weapon_current_angle = keys.angle_around_player;
// else
//   s.weapon_current_angle = keys.angle_around_player;
// // set angle, but freezes weapon angle throughout slash?
// weapon.angle_radians = keys.angle_around_player;
// // remove any other slash attacks from this player
// std::vector<Attack>::iterator it = attacks.begin();
// while (it != attacks.end()) {
//   Attack& att = (*it);
//   if (att.entity_weapon_owner_id == player_obj.id && att.weapon_type == ShopItem::SHOVEL) {
//     it = attacks.erase(it);
//   } else {
//     ++it;
//   }
// }
// if (s.slash_attack_time_left > 0.0f) {
//   s.slash_attack_time_left -= delta_time_s;
//   weapon.do_render = true;
//   weapon.do_physics = true;
//   if (s.attack_left_to_right)
//     s.weapon_current_angle += s.weapon_angle_speed;
//   else
//     s.weapon_current_angle -= s.weapon_angle_speed;
//   glm::ivec2 pos = rotate_b_around_a(player_obj, weapon, s.weapon_radius, s.weapon_current_angle);
//   s.weapon_target_pos = pos;
// } else {
//   weapon.do_physics = false;
//   s.weapon_target_pos = player_obj.pos;
// }
// // Create a new slash with unique attack.id
// Attack a = Attack(player_obj.id, weapon.id, ShopItem::SHOVEL, s.damage);
// attacks.push_back(a);
// lerp weapon to target position
//   weapon.pos = glm::lerp(glm::vec3(weapon.pos.x, weapon.pos.y, 0.0f),
//                          glm::vec3(s.weapon_target_pos.x, s.weapon_target_pos.y, 0.0f),
//                          glm::clamp(delta_time_s * s.weapon_damping, 0.0f, 1.0f));
//   return true;
// };

// // An "Attack" is basically a limiter that prevents collisions
// // applying damage on every frame. This could end up being super weird.
