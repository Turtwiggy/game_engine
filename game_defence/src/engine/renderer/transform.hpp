#pragma once

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct TagComponent
{
  std::string tag;
};

struct TransformComponent
{
  glm::vec3 position{ 0, 0, 0 };
  glm::vec3 rotation_radians = { 0, 0, 0 };
  glm::vec3 scale{ 0, 0, 0 };

  int z_index = 0;
};

inline void
to_json(nlohmann::json& j, const TransformComponent& t)
{
  j["transform"]["x"] = t.position.x;
  j["transform"]["y"] = t.position.y;
  j["transform"]["z"] = t.position.z;
  j["transform"]["r"] = t.rotation_radians.z;
  j["transform"]["sx"] = t.scale.x;
  j["transform"]["sy"] = t.scale.y;
};

inline void
from_json(const nlohmann::json& j, TransformComponent& t)
{
  j.at("transform").at("x").get_to(t.position.x);
  j.at("transform").at("y").get_to(t.position.y);
  j.at("transform").at("z").get_to(t.position.z);
  j.at("transform").at("r").get_to(t.rotation_radians.z);
  j.at("transform").at("sx").get_to(t.scale.x);
  j.at("transform").at("sy").get_to(t.scale.y);
};

} // namespace game2d