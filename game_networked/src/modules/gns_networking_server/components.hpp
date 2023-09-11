#pragma once

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <map>
#include <string>
#include <vector>

namespace game2d {

enum class ServerEvents
{
  CLIENT_DROPPED,
  CLIENT_JOINED,
};

struct ClientMessage
{
  std::string data;
  HSteamNetConnection conn;
};

struct SINGLETON_ServerComponent
{
  ISteamNetworkingSockets* interface;
  HSteamListenSocket socket;
  HSteamNetPollGroup group;

  const int max_clients = 4; // arbitrarily chosen
  std::vector<HSteamNetConnection> clients;
  // std::vector<ServerEvents> events;

  std::vector<std::string> incoming_messages;
};

} // namespace game2d