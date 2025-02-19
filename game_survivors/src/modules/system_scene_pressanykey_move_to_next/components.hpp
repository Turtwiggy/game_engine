#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_PressAnykeyScene
{
  // prevent immediately doing action when sent by other scene
  bool one_frame_buffer = true;
};

} // namespace game2d