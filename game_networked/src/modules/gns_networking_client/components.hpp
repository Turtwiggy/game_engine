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

struct SINGLETON_ClientComponent
{
  ISteamNetworkingSockets* interface;
  HSteamNetConnection connection;

  std::vector<std::string> messages;
};

} // namespace game2d