#pragma once

#include "modules/networking/components.hpp"

#include <entt/entt.hpp>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <string>
#include <vector>

namespace game2d {

void
start_client(entt::registry& r, const std::string& addr);

void
tick_client(entt::registry& r, uint64_t milliseconds_dt);

void
client_receive_messages_on_connection(SINGLETON_ClientComponent& client, std::vector<std::string>& result);

void
client_poll_connections(SINGLETON_ClientComponent& client);

} // namespace game2d