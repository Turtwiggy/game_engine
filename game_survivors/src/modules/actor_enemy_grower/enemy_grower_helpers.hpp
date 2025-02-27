#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_circle_fixture_size(entt::registry& r, entt::entity body_e, entt::entity fix_e, float new_radius_pixels);

} // namespace game2d