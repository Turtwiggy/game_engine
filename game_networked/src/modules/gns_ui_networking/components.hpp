#pragma once

#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <string>

namespace game2d {

struct SINGLETON_NetworkingUIComponent
{
  bool start_server = false;
  int server_port = 27020;

  bool start_client = false;
  std::string host = "127.0.0.1:27020";

  bool close_networking = false;
  bool start_offline = false;
  bool server_was_started = false;
  bool client_was_started = false;

  ISteamNetworkingUtils* utils;
};

} // namespace game2d