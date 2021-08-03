// header
#include "2d_vfx.hpp"
#include "2d_game_config.hpp"

#include <iostream>

namespace game2d {

namespace vfx {

// vfx death "splat"
void
spawn_death_splat(fightingengine::RandomState& rnd,
                  const GameObject2D& enemy,
                  const sprite::type s,
                  const glm::vec4 colour,
                  std::vector<GameObject2D>& ents)
{
  GameObject2D splat = gameobject::create_generic(s, colour);
  splat.do_lifecycle_timed = true;
  splat.time_alive = 45.0f; // long splat
  splat.time_alive_left = splat.time_alive;
  splat.render_size = enemy.render_size;
  splat.physics_size = splat.render_size;
  splat.pos = enemy.pos;
  splat.angle_radians = fightingengine::rand_det_s(rnd.rng, -fightingengine::PI, fightingengine::PI);

  if (enemy.damage_taken >= enemy.damage_able_to_be_taken) {
    ents.push_back(splat);
  }
}

std::vector<sprite::type>
convert_int_to_sprites(int damage)
{

  std::vector<sprite::type> numbers;

  int number = damage;
  // this iterates over number from right to left.
  // e.g. 1230 will iterate as 0, 3, 2, 1
  while (number > 0) {
    int digit = number % 10;
    number /= 10;

    if (digit == 9)
      numbers.push_back(sprite::type::NUMBER_9);
    if (digit == 8)
      numbers.push_back(sprite::type::NUMBER_8);
    if (digit == 7)
      numbers.push_back(sprite::type::NUMBER_7);
    if (digit == 6)
      numbers.push_back(sprite::type::NUMBER_6);
    if (digit == 5)
      numbers.push_back(sprite::type::NUMBER_5);
    if (digit == 4)
      numbers.push_back(sprite::type::NUMBER_4);
    if (digit == 3)
      numbers.push_back(sprite::type::NUMBER_3);
    if (digit == 2)
      numbers.push_back(sprite::type::NUMBER_2);
    if (digit == 1)
      numbers.push_back(sprite::type::NUMBER_1);
    if (digit == 0)
      numbers.push_back(sprite::type::NUMBER_0);
  }

  // now reverse numbers in to e.g. 1230
  std::reverse(numbers.begin(), numbers.end());
  return numbers;
}

void
spawn_impact_splats(fightingengine::RandomState& rnd,
                    const GameObject2D& src,
                    const GameObject2D& dst,
                    const sprite::type sprite,
                    const glm::vec4 colour,
                    int damage_amount,
                    std::vector<GameObject2D>& ents)
{
  // these splats fire off in an arc from the enemy.pos

  GameObject2D splat = gameobject::create_generic(sprite, colour);
  splat.do_lifecycle_timed = true;
  splat.time_alive = 1.0f; // short splat
  splat.time_alive_left = splat.time_alive;
  splat.speed_default = 40.0f;
  splat.speed_current = splat.speed_default;

  // vfx impact splats

  splat.colour = colour;

  int amount_of_splats = 0;
  for (int i = 0; i < amount_of_splats; i++) {

    // position
    glm::vec2 dst_pos_center = dst.pos + dst.physics_size / 2.0f;
    glm::vec2 src_pos_center = src.pos + src.physics_size / 2.0f;
    glm::vec2 distance = src_pos_center - dst_pos_center;
    glm::vec2 dir = -glm::normalize(distance);
    glm::vec2 splat_spawn_pos = dst.pos;
    splat_spawn_pos.x += dst.physics_size.x / 2.0f - splat.physics_size.x / 2.0f;
    splat_spawn_pos.y += dst.physics_size.y / 2.0f - splat.physics_size.y / 2.0f;

    // velocity
    float theta = fightingengine::rand_det_s(rnd.rng, -fightingengine::PI, fightingengine::PI);
    glm::vec2 offset_dir;
    offset_dir.x = cos(theta) * dir.x - sin(theta) * dir.y;
    offset_dir.y = sin(theta) * dir.x + cos(theta) * dir.y;
    splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;

    // spawn impact splats
    splat.pos = splat_spawn_pos;
    splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;
    ents.push_back(splat);
  }

  // vfx damage number popup

  if (damage_amount > 0) {

    // glm::vec4 colour = glm::vec4(0.8f); // damage popup colour
    splat.colour = colour;

    std::vector<sprite::type> number_sprites = convert_int_to_sprites(damage_amount);

    int amount_of_splats = 1;
    for (int i = 0; i < amount_of_splats; i++) {

      // position
      glm::vec2 dst_pos_center = dst.pos + dst.physics_size / 2.0f;
      glm::vec2 src_pos_center = src.pos + src.physics_size / 2.0f;
      glm::vec2 distance = src_pos_center - dst_pos_center;
      glm::vec2 dir = -glm::normalize(distance);
      glm::vec2 splat_spawn_pos = dst.pos;
      splat_spawn_pos.x += dst.physics_size.x / 2.0f - splat.physics_size.x / 2.0f;
      splat_spawn_pos.y += dst.physics_size.y / 2.0f - splat.physics_size.y / 2.0f;

      // velocity
      float theta = fightingengine::rand_det_s(rnd.rng, -fightingengine::PI, fightingengine::PI);
      glm::vec2 offset_dir;
      offset_dir.x = cos(theta) * dir.x - sin(theta) * dir.y;
      offset_dir.y = sin(theta) * dir.x + cos(theta) * dir.y;
      splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;

      // spawn numbers
      splat.physics_size = { 12.0f, 12.0f };
      splat.render_size = splat.physics_size;
      int text_seperation = 6;
      int offset = 0;
      for (auto& number : number_sprites) {
        splat.pos = splat_spawn_pos;
        splat.pos.x += offset;
        splat.sprite = number;
        offset += text_seperation;
        ents.push_back(splat);
      }
    }
  }
};

} // namespace vfx

} // namespace game2d