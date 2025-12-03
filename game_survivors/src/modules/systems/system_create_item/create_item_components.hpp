#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct CreateItemRequest
{
  std::string item;
  glm::vec2 position;
  std::map<std::string, std::string> extra_data;
};

} // namespace game2d