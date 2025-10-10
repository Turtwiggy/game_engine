#pragma once

#include "scene_changed_event_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_scene_changed_event__update_water_shader(entt::registry& r, const SceneChangedEvent& evt);

} // namespace game2d