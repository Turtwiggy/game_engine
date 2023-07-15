#pragma once

#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

namespace game2d {

struct SINGLETON_NetworkingUIComponent
{
  bool start_server = false;
  bool start_client = false;
  bool close_networking = false;
  bool start_offline = false;

  bool server_was_started = false;
  bool client_was_started = false;

  ISteamNetworkingUtils* utils;
};

} // namespace game2d