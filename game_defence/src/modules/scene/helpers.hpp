#pragma once

#include "components.hpp"

#include "entt/entt.hpp"

namespace game2d {

void
move_to_scene_start(entt::registry& r, const Scene s);

entt::entity
create_player(entt::registry& r);

entt::entity
create_player_ally(entt::registry& r, const entt::entity& group);

} // namespace game2d