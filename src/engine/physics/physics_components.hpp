#pragma once

#include <box2d/box2d.h>
#include <entt/fwd.hpp>
#include <glm/fwd.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace game2d {
using namespace nlohmann;

//
// Box2d uses (m) and m/s
// it works best with bodies between size of 0.1 and 1.0.
// Box2d has an upper speed limit of 120m/s
//
const float PIXELS_PER_METER = 64.0f;

struct PhysicsBodyDef
{
  bool is_bullet = false;
  bool is_static = false;
  float linear_damping = 0.0f;
  float angular_damping = 0.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PhysicsBodyDef, is_bullet, is_static, linear_damping, angular_damping);
};

struct vec2f
{
  float x;
  float y;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(vec2f, x, y);
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
  std::vector<vec2f> offset;
  std::vector<vec2f> size_in_pixels;
  // void* user_data = nullptr;

  friend void to_json(nlohmann ::json& j, const PhysicsFixtureDef& t)
  {
    j["tag"] = t.tag;
    j["type"] = t.type;
    j["is_sensor"] = t.is_sensor;
    j["density"] = t.density;
    j["friction"] = t.friction;
    j["offset"] = t.offset;
    j["size"] = t.size_in_pixels;
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
    if (j.contains("offset"))
      j.at("offset").get_to(t.offset);
    if (j.contains("size"))
      j.at("size").get_to(t.size_in_pixels);
  };
};

//

struct PhysicsBodyComponent
{
  b2BodyId bodyId;
  std::vector<entt::entity> fixtures{};
};

struct PhysicsFixtureComponent
{
  b2BodyId bodyId; // parent body
  b2ShapeId shapeId;
};

struct SINGLE_Physics
{
  b2WorldId worldId = b2_nullWorldId;
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