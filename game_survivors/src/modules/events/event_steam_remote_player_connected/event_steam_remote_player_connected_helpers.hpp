#pragma once

#include <entt/fwd.hpp>

#include "event_steam_remote_player_connected_components.hpp"

namespace game2d {

void
handle_steam_event__remote_player_connected(entt::registry& r, const RemotePlayerConnectedEvent& evt);

} // namespace game2d