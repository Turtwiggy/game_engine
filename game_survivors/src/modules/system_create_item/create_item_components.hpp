#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct CreateItemRequest
{
  std::string item;
  glm::vec2 position;
};

} // namespace game2d