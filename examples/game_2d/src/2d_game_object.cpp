// your header
#include "2d_game_object.hpp"

// c++ standard lib
#include <iostream>

namespace game2d {

glm::vec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go)
{
  return go.pos - camera.pos;
}

bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size)
{
  glm::vec2 tl_visible = glm::vec2(0.0f, 0.0f);
  glm::vec2 br_visible = screen_size;
  if (
    // left of screen
    pos.x + size.x < tl_visible.x ||
    // top of screen
    pos.y + size.y < tl_visible.y ||
    // right of screen
    pos.x > br_visible.x ||
    // bottom of screen
    pos.y > br_visible.y) {
    return true;
  }
  return false;
}

namespace gameobject {

// logic

void
update_position(GameObject2D& obj, const float delta_time_s)
{
  obj.pos += obj.velocity * delta_time_s;
}

void
update_entities_lifecycle(std::vector<GameObject2D>& objs, const float delta_time_s)
{
  std::vector<GameObject2D>::iterator it_1 = objs.begin();
  while (it_1 != objs.end()) {
    GameObject2D& obj = (*it_1);

    if (obj.do_lifecycle_timed) {
      obj.time_alive_left -= delta_time_s;
      if (obj.time_alive_left <= 0.0f) {
        obj.flag_for_delete = true;
      }
    }

    if (obj.do_lifecycle_health) {
      if (obj.hits_taken >= obj.hits_able_to_be_taken) {
        obj.flag_for_delete = true;
      }
    }

    ++it_1;
  }
}

void
erase_entities_that_are_flagged_for_delete(std::vector<GameObject2D>& objs, const float delta_time_s)
{
  std::vector<GameObject2D>::iterator it_1 = objs.begin();
  while (it_1 != objs.end()) {
    GameObject2D& obj = (*it_1);

    if (obj.flag_for_delete)
      it_1 = objs.erase(it_1);
    else
      ++it_1;
  }
}

// entities

GameObject2D
create_bullet(sprite::type sprite, int tex_slot, glm::vec4 colour)
{
  GameObject2D game_object;
  // config
  game_object.sprite = sprite;
  game_object.tex_slot = tex_slot;
  game_object.colour = colour;
  // default
  game_object.collision_layer = CollisionLayer::Bullet;
  game_object.name = "bullet";
  game_object.render_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.physics_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.speed_default = 200.0f;
  game_object.speed_current = game_object.speed_default;
  game_object.time_alive_left = 6.0f;
  game_object.do_lifecycle_timed = true;
  return game_object;
};

GameObject2D
create_camera()
{
  GameObject2D game_object;
  // default
  game_object.pos = glm::vec2{ 0.0f, 0.0f };
  return game_object;
}

GameObject2D
create_enemy(sprite::type sprite, int tex_slot, glm::vec4 colour, fightingengine::RandomState& rnd)
{
  GameObject2D game_object;
  // config
  game_object.sprite = sprite;
  game_object.tex_slot = tex_slot;
  game_object.colour = colour;
  game_object.speed_default = 60.0f;
  game_object.speed_current = game_object.speed_default;
  // default
  game_object.collision_layer = CollisionLayer::Enemy;
  game_object.name = "wall";
  game_object.angle_radians = 0.0;
  game_object.render_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.physics_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.hits_able_to_be_taken = 2;

  // roll a dice for ai
  float rand = fightingengine::rand_det_s(rnd.rng, 0.0f, 1.0f);
  if (rand <= 0.75f) {
    game_object.ai_priority_list.push_back(AiBehaviour::MOVEMENT_ARC_ANGLE);
    // locked between -89.9 and 89.9 as uses sin(theta), and after these values makes less sense
    game_object.approach_theta_degrees = fightingengine::rand_det_s(rnd.rng, -89.9f, 89.9f);
    std::cout << "approach angle: " << game_object.approach_theta_degrees << std::endl;
  } else {
    game_object.ai_priority_list.push_back(AiBehaviour::MOVEMENT_DIRECT);
    game_object.approach_theta_degrees = 0.0f;
  }

  return game_object;
};

GameObject2D
create_generic(sprite::type sprite, int tex_slot, glm::vec4 colour)
{
  GameObject2D game_object;
  game_object.collision_layer = CollisionLayer::NoCollision;
  game_object.name = "generic";
  game_object.tex_slot = tex_slot;
  game_object.sprite = sprite;
  game_object.render_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.physics_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.colour = colour;
  return game_object;
}

GameObject2D
create_tree(int tex_slot)
{
  GameObject2D game_object;
  game_object.collision_layer = CollisionLayer::Obstacle;
  game_object.name = "tree";
  game_object.tex_slot = tex_slot;
  game_object.sprite = sprite::type::EMPTY;
  game_object.render_size = { 32.0f, 32.0f };
  game_object.physics_size = { 32.0f, 32.0f };
  game_object.colour = { 0.25f, 1.0f, 0.25f, 1.0f };
  return game_object;
}

GameObject2D
create_player(sprite::type sprite, int tex_slot, glm::vec4 colour, glm::vec2 screen)
{
  GameObject2D game_object;
  // config
  game_object.sprite = sprite;
  game_object.tex_slot = tex_slot;
  game_object.colour = colour;
  game_object.pos = { screen.x / 2.0f, screen.y / 2.0f };
  // default
  game_object.collision_layer = CollisionLayer::Player;
  game_object.name = "player";
  game_object.angle_radians = 0.0;
  game_object.render_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.physics_size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.velocity = { 0.0f, 0.0f };
  game_object.velocity_boost_modifier = 2.0f;
  game_object.speed_default = 50.0f;
  game_object.speed_current = game_object.speed_default;
  game_object.invulnerable = false;
  game_object.hits_able_to_be_taken = 10;
  game_object.bullet_seconds_between_spawning = 1.0f;
  return game_object;
};

GameObject2D
create_kennynl_texture(int tex_slot)
{
  GameObject2D game_object;
  game_object.name = "texture_sheet";
  game_object.pos = { 0.0f, 20.0f };
  game_object.render_size = { 768.0f, 352.0f };
  game_object.physics_size = { 768.0f, 352.0f };
  game_object.colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  game_object.angle_radians = 0.0;
  game_object.sprite = sprite::type::EMPTY;
  return game_object;
}

} // namespace gameobject

} // namespace game2d