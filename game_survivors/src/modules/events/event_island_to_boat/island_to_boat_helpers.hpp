#pragma once

#include "island_to_boat_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
handle_island_to_boat_event__start_game(entt::registry& r, const IslandToBoatEvent& evt);

} // namespace game2d