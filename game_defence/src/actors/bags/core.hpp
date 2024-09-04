#pragma once

#include "colour/colour.hpp"

#include <glm/glm.hpp>

#include <string>

namespace game2d {

const glm::ivec3 DEFAULT_SIZE{ 32, 32, 1 };
const glm::ivec3 HALF_SIZE{ 16, 16, 1 };
const glm::ivec2 SMALL_SIZE{ 4, 4 };

struct EntityData
{
  glm::vec2 pos{ 0, 0 };
  glm::vec2 size = DEFAULT_SIZE;
  std::string sprite = "EMPTY";
  std::string icon = "";
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace game2d