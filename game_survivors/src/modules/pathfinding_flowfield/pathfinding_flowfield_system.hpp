#pragma once

#include "modules/pathfinding_flowfield/pathfinding_flowfield_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
generate_flowfield(entt::registry& r, const GenerateFlowfieldParams in);

void
update_generate_flow_field_system(entt::registry& r, glm::vec2 mouse_pos);

void
update_display_flow_field_system(entt::registry& r);

} // namespace game2d