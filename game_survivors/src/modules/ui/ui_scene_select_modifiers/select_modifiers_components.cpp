#include "pch.hpp"

#include "select_modifiers_components.hpp"

namespace game2d {

void
Option_Rocks::update(entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 0, 1); // true or false.

  // update value.
  populate_rocks = (bool)hindex;
}

} // namespace game2d