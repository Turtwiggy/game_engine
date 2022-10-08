#pragma once

#include "game/components/app.hpp"

namespace game2d {

struct FovComponent
{
  bool placeholder = true;
};

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
update_tile_fov_system(GameEditor& editor, Game& game);

} // game2d