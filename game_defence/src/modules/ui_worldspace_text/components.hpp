#pragma once

#include <glm/glm.hpp>

#include <string>

namespace game2d {

struct WorldspaceTextComponent
{
  std::string text;
  glm::ivec2 offset{ 0, 0 };
};

} // namespace game2d