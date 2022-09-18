#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct VisibleComponent
{
  bool placeholder = true;
};

struct NotVisibleComponent
{
  bool placeholder = true;
};

struct NotVisibleButPreviouslySeenComponent
{
  bool placeholder = true;
};

void
update_tile_fov_system(entt::registry& r);

} // game2d