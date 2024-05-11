#pragma once

#include "colour/colour.hpp"
#include <entt/entt.hpp>

#include <set>
#include <vector>

namespace game2d {

struct HoverableComponent
{
  bool placeholder = true;
};

struct HoveredComponent
{
  bool placeholder = true;
};

struct SelectedComponent
{
  bool placeholder = true;
};

struct DefaultColour
{
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct HoveredColour
{
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace game2d