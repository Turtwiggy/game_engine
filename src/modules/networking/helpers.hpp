#pragma once

#include "components.hpp"

#include <entt/entt.hpp>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <map>

namespace game2d {

void
InitSteamDatagramConnectionSockets();

void
SendStringToClient(ISteamNetworkingSockets* interface, HSteamNetConnection conn, const char* str);

void
SendStringToAllClients(ISteamNetworkingSockets* interface,
                       std::map<HSteamNetConnection, Client>& clients,
                       const char* str,
                       HSteamNetConnection except = k_HSteamNetConnection_Invalid);

void
OnSteamNetConnectionStatusChanged(entt::registry& r, SteamNetConnectionStatusChangedCallback_t* info);

}; // namespace game2d