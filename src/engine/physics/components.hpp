#pragma once

#include <box2d/b2_fixture.h>
#include <box2d/b2_math.h>
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace game2d {
using namespace nlohmann;

struct PhysicsBodyDef
{
  bool is_bullet = false;
  bool is_static = false;
  float linear_damping = 0.0f;
  float angular_damping = 0.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PhysicsBodyDef, is_bullet, is_static, linear_damping, angular_damping);
};

struct PhysicsFixtureDef
{
  // required...
  std::string tag = "unknown";
  std::string type = "unknown"; // box or circle
  bool is_sensor = false;

  // optionals...
  float density = 1.0f;
  float friction = 0.0f;
  float restitution = 1.0f;

  // optionals... for circle
  float radius = 0.0f;

  friend void to_json(nlohmann ::json& j, const PhysicsFixtureDef& t)
  {
    j["tag"] = t.tag;
    j["type"] = t.type;
    j["is_sensor"] = t.is_sensor;
    j["density"] = t.density;
    j["friction"] = t.friction;
    j["restitution"] = t.restitution;
    j["radius"] = t.radius;
  }
  friend void from_json(const nlohmann ::json& j, PhysicsFixtureDef& t)
  {
    j.at("tag").get_to(t.tag);
    j.at("type").get_to(t.type);
    j.at("is_sensor").get_to(t.is_sensor);
    if (j.contains("density"))
      j.at("density").get_to(t.density);
    if (j.contains("friction"))
      j.at("friction").get_to(t.friction);
    if (j.contains("restitution"))
      j.at("restitution").get_to(t.restitution);
    if (j.contains("radius"))
      j.at("radius").get_to(t.radius);
  };
};

//

struct PhysicsBodyComponent
{
  b2Body* body = nullptr;
  float base_speed = 10; // influences linear velocity

  std::vector<entt::entity> fixtures{};
};

struct PhysicsFixtureComponent
{
  b2Body* body = nullptr; // parent body
  b2Fixture* fixture = nullptr;
};

struct SINGLE_Physics
{
  b2World* world;
};

struct SeparateTransformAndAABB
{
  bool placeholder = true;
};

struct SetTransformRotationBasedOnPhysicsBody
{
  bool placeholder = true;
};

struct SetTransformRotationBasedOnPhysicsVelocity
{
  bool placeholder = true;
};

} // namespace game2d