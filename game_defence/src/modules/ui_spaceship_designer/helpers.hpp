#pragma once

#include <entt/entt.hpp>

namespace game2d {

entt::entity
create_shotgun(entt::registry& r, entt::entity parent);

entt::entity
create_jetpack_player(entt::registry& r);

} // namespace game2d