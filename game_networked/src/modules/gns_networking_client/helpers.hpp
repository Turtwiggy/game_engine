#pragma once

#include "components.hpp"

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace game2d {

void
start_client(entt::registry& r, const std::string& addr, uint16 port);

void
tick_client(entt::registry& r, uint64_t milliseconds_dt);

//
// Client -> Server
//

void
send_message_to_server(const SINGLETON_ClientComponent& client, const std::string& packet);

} // namespace game2d