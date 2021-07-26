// header
#include "2d_vfx.hpp"

namespace game2d {

namespace vfx {

// vfx death "splat"
void
spawn_death_splat(fightingengine::RandomState& rnd,
                  const GameObject2D& enemy,
                  const sprite::type s,
                  const int tex_unit,
                  const glm::vec4 colour,
                  std::vector<GameObject2D>& ents)
{
  GameObject2D splat = gameobject::create_generic(s, tex_unit, colour);
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

void
spawn_impact_splats(fightingengine::RandomState& rnd,
                    const GameObject2D& enemy,
                    const GameObject2D& player,
                    const sprite::type s,
                    const int tex_unit,
                    const glm::vec4 colour,
                    std::vector<GameObject2D>& ents)
{
  GameObject2D splat = gameobject::create_generic(s, tex_unit, colour);
  splat.do_lifecycle_timed = true;

  // these splats fire off in an arc from the enemy.pos
  splat.time_alive = 0.5f; // short splat
  splat.time_alive_left = splat.time_alive;
  splat.colour = colour;
  splat.speed_default = 40.0f;
  splat.speed_current = splat.speed_default;
  splat.physics_size = { 10.0f, 10.0f };
  splat.render_size = splat.physics_size;

  int amount_of_splats = 5;
  for (int i = 0; i < amount_of_splats; i++) {

    glm::vec2 enemy_pos_center = enemy.pos + enemy.physics_size / 2.0f;
    glm::vec2 player_pos_center = player.pos + player.physics_size / 2.0f;
    glm::vec2 distance = player_pos_center - enemy_pos_center;
    glm::vec2 dir = -glm::normalize(distance);

    glm::vec2 splat_spawn_pos = enemy.pos;
    splat_spawn_pos.x += enemy.physics_size.x / 2.0f - splat.physics_size.x / 2.0f;
    splat_spawn_pos.y += enemy.physics_size.y / 2.0f - splat.physics_size.y / 2.0f;

    splat.pos = splat_spawn_pos;

    float theta = fightingengine::rand_det_s(rnd.rng, -fightingengine::PI, fightingengine::PI);
    glm::vec2 offset_dir;
    offset_dir.x = cos(theta) * dir.x - sin(theta) * dir.y;
    offset_dir.y = sin(theta) * dir.x + cos(theta) * dir.y;

    splat.velocity = glm::normalize(dir + glm::normalize(offset_dir)) * splat.speed_current;

    ents.push_back(splat);
  }
};

} // namespace vfx

} // namespace game2d