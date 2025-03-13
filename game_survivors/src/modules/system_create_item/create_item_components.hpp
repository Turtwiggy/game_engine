#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct CreateItemRequest
{
  std::string item;
  glm::vec2 position;
};

} // namespace game2d