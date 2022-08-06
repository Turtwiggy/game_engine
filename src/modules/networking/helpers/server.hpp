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
start_server_or_quit(entt::registry& r, int port);

void
server_receive_messages_on_poll_group(SINGLETON_ServerComponent& server, std::vector<ClientMessage>& result);

void
server_poll_connections(SINGLETON_ServerComponent& server);

void
tick_server(entt::registry& r, uint64_t milliseconds_dt);

//
// Server -> Client
//

void
send_string_to_client(ISteamNetworkingSockets* interface, HSteamNetConnection conn, const std::string& str);

void
send_string_to_all_clients(ISteamNetworkingSockets* interface,
                           std::vector<HSteamNetConnection>& clients,
                           const char* str,
                           HSteamNetConnection except = k_HSteamNetConnection_Invalid);

} // namespace game2d