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

// struct Client
// {
//   std::string nickname;
// };

enum class ServerEvents
{
  CLIENT_DROPPED,
  CLIENT_JOINED,
};

struct SINGLETON_ClientComponent
{
  ISteamNetworkingSockets* interface;
  HSteamNetConnection connection;
};

struct SINGLETON_ServerComponent
{
  int process_fixed_frame = 0;

  ISteamNetworkingSockets* interface;
  HSteamListenSocket socket;
  HSteamNetPollGroup group;

  const int max_clients = 4; // arbitrarily chosen
  std::vector<HSteamNetConnection> clients;
  std::vector<ServerEvents> events;
};

} // namespace game2d