// your header
#include "2d_game_object.hpp"

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
update_position(GameObject2D& obj, float delta_time_s)
{
  obj.pos += obj.velocity * delta_time_s;
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
  game_object.size = { 20.0f, 20.0f };
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
create_enemy(sprite::type sprite, int tex_slot, glm::vec4 colour)
{
  GameObject2D game_object;
  // config
  game_object.sprite = sprite;
  game_object.tex_slot = tex_slot;
  game_object.colour = colour;
  // default
  game_object.collision_layer = CollisionLayer::Enemy;
  game_object.name = "wall";
  game_object.angle_radians = 0.0;
  game_object.size = { 20.0f, 20.0f };
  game_object.hits_able_to_be_taken = 3;
  return game_object;
};

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
  game_object.size = { 1.0f * 768.0f / 48.0f, 1.0f * 362.0f / 22.0f };
  game_object.velocity = { 0.0f, 0.0f };
  game_object.velocity_boost_modifier = 2.0f;
  game_object.speed_default = 50.0f;
  game_object.speed_current = game_object.speed_default;
  game_object.invulnerable = true;
  game_object.hits_able_to_be_taken = 10;
  return game_object;
};

} // namespace gameobject

} // namespace game2d