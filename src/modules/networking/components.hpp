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
  ISteamNetworkingSockets* interface;
  HSteamNetConnection connection;
};

struct SINGLETON_ServerComponent
{
  ISteamNetworkingSockets* interface;
  HSteamListenSocket socket;
  HSteamNetPollGroup group;
  std::map<HSteamNetConnection, Client> clients;
};

} // namespace game2d