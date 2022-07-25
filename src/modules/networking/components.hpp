#pragma once

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <map>
#include <string>

namespace game2d {

struct Client
{
  std::string nickname;
};

struct SINGLETON_ClientComponent
{
  const uint16 port = 27020;

  ISteamNetworkingSockets* interface;
  HSteamNetConnection connection;
};

struct SINGLETON_ServerComponent
{
  const uint16 port = 27020;

  ISteamNetworkingSockets* interface;
  HSteamListenSocket socket;
  HSteamNetPollGroup group;
  std::map<HSteamNetConnection, Client> clients;
};

} // namespace game2d