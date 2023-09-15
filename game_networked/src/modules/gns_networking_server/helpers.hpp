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
start_server_or_quit(entt::registry& r, uint16 port);

void
tick_server(entt::registry& r, uint64_t milliseconds_dt);

void
send_string_to_client(ISteamNetworkingSockets* interface, HSteamNetConnection conn, const std::string& str);

void
send_string_to_all_clients(const SINGLETON_ServerComponent& server, const std::string& s);

} // namespace game2d