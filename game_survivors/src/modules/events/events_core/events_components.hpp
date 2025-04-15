#pragma once

#include <entt/fwd.hpp>

#include <functional>
#include <vector>

namespace game2d {

struct SINGLE_Events
{
  entt::dispatcher* dispatcher;
};

struct SINGLE_PostFixedUpdateCallbacks
{
  std::vector<std::function<void(entt::registry&)>> callbacks;
};

} // namespace game2d