#pragma once

namespace game2d {

struct WasCollidedWithComponent
{
  bool placeholder = true;
};

struct CollidingWithCursorComponent
{
  bool placeholder = true;
};

struct CollidingWithExitComponent
{
  entt::entity exit;
};

} // namespace game2d