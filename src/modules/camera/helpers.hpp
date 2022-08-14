#pragma once

#include <entt/entt.hpp>

namespace game2d {

entt::entity
create_camera(entt::registry& r);

entt::entity
get_main_camera(entt::registry& registry);

} // namespace game2d