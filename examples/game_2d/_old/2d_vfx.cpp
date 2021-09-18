// header
#include "2d_vfx.hpp"
#include "constants.hpp"

#include <iostream>

namespace game2d {

namespace vfx {

// vfx death "splat"
void
spawn_death_splat(engine::RandomState& rnd,
                  const GameObject2D& enemy,
                  const sprite::type s,
                  std::vector<GameObject2D>& ents)
{
  GameObject2D splat = gameobject::create_generic();
  splat.sprite = s;
  splat.do_lifecycle_timed = true;
  splat.time_alive = 45.0f; // long splat
  splat.time_alive_left = splat.time_alive;
  splat.render_size = enemy.render_size;
  splat.physics_size = splat.render_size;
  splat.pos = enemy.pos;
  splat.angle_radians = engine::rand_det_s(rnd.rng, -engine::PI, engine::PI);
  splat.colour = enemy_death_splat_colour;

  if (enemy.damage_taken >= enemy.damage_able_to_be_taken) {
    ents.push_back(splat);
  }
}

void
spawn_impact_splats(engine::RandomState& rnd,
                    const GameObject2D& src,
                    const GameObject2D& dst,
                    const sprite::type sprite,
                    int damage_amount,
                    std::vector<GameObject2D>& ents)
{
  // these splats fire off in an arc from the enemy.pos

  GameObject2D splat = gameobject::create_generic();
  splat.sprite = sprite;
  splat.do_lifecycle_timed = true;
  splat.time_alive = 1.0f; // short splat
  splat.time_alive_left = splat.time_alive;
  splat.speed_default = 40.0f;
  splat.speed_current = splat.speed_default;

  // vfx impact splats

  // int amount_of_splats = 0;
  // for (int i = 0; i < amount_of_splats; i++) {

  //   splat.colour = damage_number_colour;

  //   // position
  //   glm::vec2 half_dst = glm::vec2(dst.physics_size.x / 2.0f, dst.physics_size.y / 2.0f);
  //   glm::vec2 half_src = glm::vec2(dst.physics_size.x / 2.0f, dst.physics_size.y / 2.0f);
  //   glm::ivec2 dst_pos_center = dst.pos + glm::ivec2(int(half_dst.x), int(half_dst.y));
  //   glm::ivec2 src_pos_center = src.pos + glm::ivec2(int(half_src.x), int(half_src.y));
  //   glm::vec2 distance = src_pos_center - dst_pos_center;
  //   glm::vec2 dir = -glm::normalize(distance);
  //   glm::vec2 splat_spawn_pos = dst.pos;
  //   splat_spawn_pos.x += dst.physics_size.x / 2.0f - splat.physics_size.x / 2.0f;
  //   splat_spawn_pos.y += dst.physics_size.y / 2.0f - splat.physics_size.y / 2.0f;

  //   // velocity
  //   float theta = engine::rand_det_s(rnd.rng, -engine::PI, engine::PI);
  //   glm::vec2 offset_dir;
  //   offset_dir.x = cos(theta) * dir.x - sin(theta) * dir.y;
  //   offset_dir.y = sin(theta) * dir.x + cos(theta) * dir.y;
  //   splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;

  //   // spawn impact splats
  //   splat.pos = splat_spawn_pos;
  //   splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;
  //   ents.push_back(splat);
  // }

  // vfx damage number popup

  if (damage_amount > 0) {

    splat.colour = damage_number_colour;

    std::vector<sprite::type> number_sprites = convert_int_to_sprites(damage_amount);

    int amount_of_splats = 1;
    for (int i = 0; i < amount_of_splats; i++) {

      // position
      glm::vec2 half_dst = glm::vec2(dst.physics_size.x / 2.0f, dst.physics_size.y / 2.0f);
      glm::vec2 half_src = glm::vec2(dst.physics_size.x / 2.0f, dst.physics_size.y / 2.0f);
      glm::ivec2 dst_pos_center = dst.pos + glm::ivec2(int(half_dst.x), int(half_dst.y));
      glm::ivec2 src_pos_center = src.pos + glm::ivec2(int(half_src.x), int(half_src.y));
      glm::vec2 distance = src_pos_center - dst_pos_center;
      glm::vec2 dir = -glm::normalize(distance);
      glm::vec2 splat_spawn_pos = dst.pos;
      splat_spawn_pos.x += dst.physics_size.x / 2.0f - splat.physics_size.x / 2.0f;
      splat_spawn_pos.y += dst.physics_size.y / 2.0f - splat.physics_size.y / 2.0f;

      // velocity
      float theta = engine::rand_det_s(rnd.rng, -engine::PI, engine::PI);
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