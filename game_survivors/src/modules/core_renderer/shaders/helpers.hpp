#pragma once

#include "engine/entt/helpers.hpp"
#include "modules/core_renderer/components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
update_lights(entt::registry& r, SINGLE_RendererInfo& ri);

} // namespace game2d