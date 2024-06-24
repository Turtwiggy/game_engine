#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct WantsToSprint
{
  bool placeholder = true;
};

struct WantsToReleaseSprint
{
  bool placeholder = true;
};

struct SprintComponent
{
  float sprint_max_seconds = 0.0f;
  float sprint_left_seconds = 0.0f;
  bool is_sprinting = false;
};

} // namespace game2d