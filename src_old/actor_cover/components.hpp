#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct CoverComponent
{
  bool placeholder = true;
};

// each piece of cover works out what's in cover next to it,
// and adds this component
struct InCoverComponent
{
  bool placeholder = true;
};

} // namespace game2d