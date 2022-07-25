#include "helpers.hpp"

#include <assert.h>
#include <iostream>

namespace game2d {

void
DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
{
  printf("%s\n", pszMsg);
  if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
    fflush(stdout);
    fflush(stderr);
    exit(0);
  }
};

void
InitSteamDatagramConnectionSockets()
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

  SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

void
SendStringToClient(ISteamNetworkingSockets* interface, HSteamNetConnection conn, const char* str)
{
  interface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void
SendStringToAllClients(ISteamNetworkingSockets* interface,
                       std::map<HSteamNetConnection, Client>& clients,
                       const char* str,
                       HSteamNetConnection except)
{
  for (auto& c : clients) {
    if (c.first != except)
      SendStringToClient(interface, c.first, str);
  }
}

// void
// OnSteamNetConnectionStatusChanged(entt::registry& r, SteamNetConnectionStatusChangedCallback_t* info)
// {
//   assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

//   char temp[1024];

//   // What's the state of the connection?
//   switch (info->m_info.m_eState) {
//     case k_ESteamNetworkingConnectionState_None:
//       // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
//       break;
//     case k_ESteamNetworkingConnectionState_ClosedByPeer:
//     case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
//       // Ignore if they were not previously connected.  (If they disconnected
//       // before we accepted the connection.)
//       if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
//         // Locate the client.  Note that it should have been found, because this
//         // is the only codepath where we remove clients (except on shutdown),
//         // and connection change callbacks are dispatched in queue order.
//         // auto itClient = m_mapClients.find(info->m_hConn);
//         // assert(itClient != m_mapClients.end());
//       }
//     }
//   }
// }

} // namespace game2d