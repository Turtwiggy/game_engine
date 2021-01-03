
// c++ lib headers
#include <iostream>

// other lib headers
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#ifdef WIN32
#include <windows.h> // Ug, for NukeProcess -- see below
#else
#include <signal.h>
#include <unistd.h>
#endif

// // engine
// #include "engine/networking/test.hpp"

// static void
// InitSteamDatagramConnectionSockets()
// {
// #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
//   SteamDatagramErrMsg errMsg;
//   if (!GameNetworkingSockets_Init(nullptr, errMsg))
//     FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
// #else
//   SteamDatagramClient_SetAppID(570); // Just set something, doesn't matter what
//   // SteamDatagramClient_SetUniverse( k_EUniverseDev );

//   SteamDatagramErrMsg errMsg;
//   if (!SteamDatagramClient_Init(true, errMsg))
//     FatalError("SteamDatagramClient_Init failed.  %s", errMsg);

//   // Disable authentication when running with Steam, for this example, since we're
//   not a
//   // real app.
//   // Authentication is disabled automatically in the open-sourceversion since we
//   don't
//   // have a trusted third party to issue certs.
//   SteamNetworkingUtils()->SetGlobalConfigValueInt32(
//     k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
// #endif

//   g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

//   SteamNetworkingUtils()->SetDebugOutputFunction(
//     k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
// }

// static void
// ShutdownSteamDatagramConnectionSockets()
// {
//   // Give connections time to finish up.  This is an application layer protocol
//   // here, it's not TCP.  Note that if you have an application and you need to be
//   // more sure about cleanup, you won't be able to do this.  You will need to send
//   // a message and then either wait for the peer to close the connection, or
//   // you can pool the connection to see if any reliable data is pending.
//   std::this_thread::sleep_for(std::chrono::milliseconds(500));

// #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
//   GameNetworkingSockets_Kill();
// #else
//   SteamDatagramClient_Kill();
// #endif
// }

int
main()
{
  printf("starting server... \n");

  // fightingengine::random_function();

  printf("exit server... \n");
  return 0;
}