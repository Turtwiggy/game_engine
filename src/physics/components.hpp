#pragma once

#include <box2d/b2_math.h>
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

//

struct PhysicsDescription
{
  b2BodyType type = b2_staticBody;
  bool is_bullet = false;
  bool is_sensor = false;
  float density = 1.0f;
  float angular_damping = 0.0f;

  // set when creating body...
  glm::vec2 position{ 0, 0 };
  glm::vec2 size{ 16, 16 };
};

//

struct PhysicsBodyComponent
{
  b2Body* body;
  float base_speed = 10; // influences linear velocity
};

struct SINGLE_Physics
{
  b2World* world;
};

} // namespace game2d