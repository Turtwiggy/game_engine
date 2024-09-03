#pragma once

#include "colour/colour.hpp"

#include <glm/glm.hpp>

#include <string>

namespace game2d {

struct EntityData
{
  glm::vec2 pos{ 0, 0 };
  glm::vec2 size{ 32, 32 }; // default size
  std::string sprite = "EMPTY";
  std::string icon = "";
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace game2d