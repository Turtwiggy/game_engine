#pragma once

#include "modules/gen_dungeons/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/lights/components.hpp"
#include "renderer/transform.hpp"

#include <vector>

namespace game2d {

void
light_up_room(std::vector<Light>& lights,
              int& i,
              const Room& room,
              const SINGLETON_RendererInfo& ri,
              const TransformComponent& camera_t);

} // namespace game2d