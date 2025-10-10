#pragma once

#include <entt/fwd.hpp>
#include <functional>

namespace game2d {

struct InteractableComponent
{
  bool placeholder = true;

  std::function<void()> action = []() {};
};

} // namespace game2d