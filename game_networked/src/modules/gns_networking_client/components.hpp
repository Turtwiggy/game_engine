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
  int fixed_frame = 0;
  int simulation_frame = 0;

  ISteamNetworkingSockets* interface;
  HSteamNetConnection connection;
};

} // namespace game2d