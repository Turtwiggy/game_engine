#include "items.hpp"

#include "game/components/breakable.hpp"
#include "game/create_entities.hpp"

#include <iostream> // temp
#include <memory>

namespace game2d {

bool
Potion::use(entt::registry& r, std::vector<entt::entity>& entities)
{
  std::cout << "healin..!" << std::endl;

  if (entities.size() > 0) {
    auto entity = entities[0]; // potion, only apply to first atm

    if (r.any_of<BreakableComponent>(entity)) {
      auto& hp = r.get<BreakableComponent>(entity);
      hp.hp += heal_amount;
      return true;
    }
  }

  return false;
};

bool
Gun::use(entt::registry& r, std::vector<entt::entity>& entities)
{
  std::cout << "shootin..!" << std::endl;

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

  // // add some randomness to the bullet's direction
  // // todo: replace complete randomness with a recoil factor.
  // constexpr float angle_min_max = 10.0f * engine::PI / 180.0f;
  // const float random_angle = engine::rand_det_s(rnd.rng, -angle_min_max, angle_min_max);

  // Create an attack ID
  // std::cout << "bullet attack, attack id: " << a.id << std::endl;
  // Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
  // gs.attacks.push_back(a);

  return true;
};

bool
Sword::use(entt::registry& r, std::vector<entt::entity>& entities)
{
  std::cout << "slashin..!" << std::endl;

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

  return true;
};

bool
Turret::use(entt::registry& r, std::vector<entt::entity>& entities)
{
  return true;
};

} // namespace game2d