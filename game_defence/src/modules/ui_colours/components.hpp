#pragma once

#include "colour/colour.hpp"

#include <string>
#include <vector>

namespace game2d {

struct ColourTagged
{
  std::string name;
  engine::SRGBColour colour;
  std::vector<std::string> tags;
};

struct SINGLE_ColoursInfo
{
  std::vector<ColourTagged> colours;
};

} // namespace game2d