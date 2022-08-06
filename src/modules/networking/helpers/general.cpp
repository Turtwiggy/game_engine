#include "general.hpp"

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <iostream>
#include <stdio.h>

namespace game2d {

void
debug_output(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
  printf("%s\n", pszMsg);
  if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
    fflush(stdout);
    fflush(stderr);
    exit(0);
  }
};

void
init_steam_datagram_connection_sockets()
{
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  std::cout << "(networking) STEAMNETWORKINGSOCKETS_OPENSOURCE " << std::endl;

  SteamDatagramErrMsg err;
  if (!GameNetworkingSockets_Init(nullptr, err)) {
    std::cerr << "Failed to init gamenetworkingsockets" << std::endl;
    exit(0);
  }

#else
  std::cout << "GAMENETWORKINGSOCKETS init..." << std::endl;

  // SteamDatagram_SetAppID(570); // Just set something, doesn't matter what
  // SteamDatagram_SetUniverse(false, k_EUniverseDev);
  // SteamDatagramErrMsg err;
  // if (!SteamDatagramClient_Init(err))
  //   std::cerr << "Failed to init SteamDatagramClient_Init" << std::endl;

  // Disable authentication when running with Steam, for this
  // example, since we're not a real app.
  // Authentication is disabled automatically in the open-source
  // version since we don't have a trusted third party to issue
  // certs.
  // SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif

  SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, debug_output);
}

// haha you wanted to close networking and now the app crashed
void
close_networking()
{
  // TODO: close connections, then do below

  // TODO: do this?
  // #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  //     GameNetworkingSockets_Kill();
  // #else
  //     SteamDatagramClient_Kill();
  // #endif
  exit(0);
}

} // namespace game2d