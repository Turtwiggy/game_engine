#pragma once

#include "components.hpp"

#include <entt/entt.hpp>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <map>
#include <string>

namespace game2d {

void
init_steam_datagram_connection_sockets();

void
start_server_or_quit(entt::registry& r, int port, void* callback);

void
start_client(entt::registry& r, const std::string& addr, void* callback);

void
close_networking();

void
send_string_to_client(ISteamNetworkingSockets* interface, HSteamNetConnection conn, const std::string& str);

void
send_string_to_all_clients(ISteamNetworkingSockets* interface,
                           std::map<HSteamNetConnection, Client>& clients,
                           const char* str,
                           HSteamNetConnection except = k_HSteamNetConnection_Invalid);

void
OnSteamNetConnectionStatusChanged(entt::registry& r, SteamNetConnectionStatusChangedCallback_t* info);

}; // namespace game2d