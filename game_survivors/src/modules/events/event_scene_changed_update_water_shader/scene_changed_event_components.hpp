#pragma once

#include "modules/scene/scene_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct SceneChangedEvent
{
  Scene old_scene;
  Scene new_scene;
};

} // namespace game2d