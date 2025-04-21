#pragma once

#include "modules/core/renderer/fluidsim/components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
load_fluidsim(entt::registry& r, FluidSimData& data, int used_tex_units);

void
setup_fluidsim_update(entt::registry& r);

void
rebind_fluidsim(entt::registry& r, FluidSimData& data, int used_tex_units);

} // namespace game2d