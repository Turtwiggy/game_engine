#pragma once

// your project headers
#include "systems/spritemap.hpp"

// other project headers
#include <glm/glm.hpp>

namespace game2d {

enum class CollisionLayer
{
  Default = 0,
  Player = 1,
  Bullet = 2,
  Destroyable = 3,

  Count = 4
};

struct GameObject2D
{
  static inline uint32_t global_int_counter = 0;
  uint32_t id = 0;
  std::string name = "DEFAULT";
  sprite::type sprite = sprite::type::SQUARE;
  int tex_slot = 0;
  CollisionLayer collision_layer = CollisionLayer::Default;
  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  float angle_radians = 0.0f;
  glm::vec2 size = { 20.0f, 20.0f };
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  glm::vec2 velocity = { 0.0f, 0.0f };
  float velocity_boost_modifier = 2.0f;
  float time_alive_left = 5.0f;
  int hits_able_to_be_taken = 3;
  int hits_taken = 0;
  bool invulnerable = false;
  float speed_current = 50.0f;
  float speed_default = 50.0f;

  GameObject2D() { id = ++GameObject2D::global_int_counter; }

  GameObject2D(const GameObject2D& obj)
  {
    id = ++GameObject2D::global_int_counter;
    name = obj.name;
    sprite = obj.sprite;
    tex_slot = obj.tex_slot;
    collision_layer = obj.collision_layer;
    pos = obj.pos;
    angle_radians = obj.angle_radians;
    size = obj.size;
    colour = obj.colour;
    velocity = obj.velocity;
    velocity_boost_modifier = obj.velocity_boost_modifier;
    time_alive_left = obj.time_alive_left;
    hits_able_to_be_taken = obj.hits_able_to_be_taken;
    hits_taken = obj.hits_taken;
    invulnerable = obj.invulnerable;
    speed_current = obj.speed_current;
    speed_default = obj.speed_default;
  }
};

[[nodiscard]] glm::vec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go);

[[nodiscard]] bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size);

} // namespace game2d
