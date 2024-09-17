#pragma once

#include "engine/entt/helpers.hpp"
#include "modules/renderer/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_stars_shader(entt::registry& r, SINGLE_RendererInfo& ri, bool in_stars_scene, float dt);

void
update_lights(entt::registry& r, SINGLE_RendererInfo& ri);

} // namespace game2d