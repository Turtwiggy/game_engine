#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct CollidingWithCursorComponent
{
  bool placeholder = true;
};

struct CollidingWithExitComponent
{
  entt::entity exit;
};

} // namespace game2d