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

  // metadata

  static inline uint32_t global_int_counter = 0;
  uint32_t id = 0;
  std::string name = "DEFAULT";

  // rendering

  sprite::type sprite = sprite::type::SQUARE;
  int tex_slot = 0;

  // physics

  CollisionLayer collision_layer = CollisionLayer::Default;

  // game

  glm::vec2 pos = { 0.0f, 0.0f }; // in pixels, centered
  float angle_radians = 0.0f;
  glm::vec2 size = { 20.0f, 20.0f };
  glm::vec4 colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  glm::vec2 velocity = { 0.0f, 0.0f };
  float velocity_boost_modifier = 2.0f;

  // lifecycle

  float time_alive_left = 5.0f;

  // hittable

  int hits_able_to_be_taken = 3;
  int hits_taken = 0;
  bool invulnerable = false;

  // speed

  float speed_current = 50.0f;
  float speed_default = 50.0f;

  GameObject2D() { id = ++GameObject2D::global_int_counter; }
};

[[nodiscard]] glm::vec2
gameobject_in_worldspace(const GameObject2D& camera, const GameObject2D& go);

[[nodiscard]] bool
gameobject_off_screen(glm::vec2 pos, glm::vec2 size, const glm::ivec2& screen_size);

} // namespace game2d
