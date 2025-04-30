#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
update_circle_fixture_size(entt::registry& r, entt::entity body_e, entt::entity fix_e, float diameter_pixels);

} // namespace game2d