
// c++ lib headers
#include <algorithm>
#include <assert.h>
#include <cctype>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>

#define STEAMNETWORKINGSOCKETS_OPENSOURCE
#include "engine/networking/net_common.hpp"
using namespace net_common;

class ChatClient
{
public:
  void Run(const SteamNetworkingIPAddr& serverAddr)
  {
    // Select instance to use.  For now we'll always use the default.
    m_pInterface = SteamNetworkingSockets();

    // Start connecting
    char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
    serverAddr.ToString(szAddr, sizeof(szAddr), true);
    Printf("Connecting to chat server at %s", szAddr);
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);
    m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &opt);
    if (m_hConnection == k_HSteamNetConnection_Invalid)
      FatalError("Failed to create connection");

    while (!g_bQuit) {
      PollIncomingMessages();
      PollConnectionStateChanges();
      PollLocalUserInput();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

private:
  HSteamNetConnection m_hConnection;
  ISteamNetworkingSockets* m_pInterface;

  void PollIncomingMessages()
  {
    while (!g_bQuit) {
      ISteamNetworkingMessage* pIncomingMsg = nullptr;
      int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection, &pIncomingMsg, 1);
      if (numMsgs == 0)
        break;
      if (numMsgs < 0)
        FatalError("Error checking for messages");

      // Just echo anything we get from the server
      fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
      fputc('\n', stdout);

      // We don't need this anymore.
      pIncomingMsg->Release();
    }
  }

  void PollLocalUserInput()
  {
    std::string cmd;
    while (!g_bQuit && LocalUserInput_GetNext(cmd)) {

      // Check for known commands
      if (strcmp(cmd.c_str(), "/quit") == 0) {
        g_bQuit = true;
        Printf("Disconnecting from chat server");

        // Close the connection gracefully.
        // We use linger mode to ask for any remaining reliable data
        // to be flushed out.  But remember this is an application
        // protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
        m_pInterface->CloseConnection(m_hConnection, 0, "Goodbye", true);
        break;
      }

      // Anything else, just send it to the server and let them parse it
      m_pInterface->SendMessageToConnection(
        m_hConnection, cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable, nullptr);
    }
  }

  void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
  {
    assert(pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid);

    // What's the state of the connection?
    switch (pInfo->m_info.m_eState) {
      case k_ESteamNetworkingConnectionState_None:
        // NOTE: We will get callbacks here when we destroy connections.  You can ignore
        // these.
        break;

      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        g_bQuit = true;

        // Print an appropriate message
        if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
          // Note: we could distinguish between a timeout, a rejected connection,
          // or some other transport problem.
          Printf("We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug);
        } else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
          Printf("Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug);
        } else {
          // NOTE: We could check the reason code for a normal disconnection
          Printf("The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug);
        }

        // Clean up the connection.  This is important!
        // The connection is "closed" in the network sense, but
        // it has not been destroyed.  We must close it on our end, too
        // to finish up.  The reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0's.
        m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        m_hConnection = k_HSteamNetConnection_Invalid;
        break;
      }

      case k_ESteamNetworkingConnectionState_Connecting:
        // We will get this callback when we start connecting.
        // We can ignore this.
        break;

      case k_ESteamNetworkingConnectionState_Connected:
        Printf("Connected to server OK");
        break;

      default:
        // Silences -Wswitch
        break;
    }
  }

  static ChatClient* s_pCallbackInstance;
  static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
  {
    s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
  }

  void PollConnectionStateChanges()
  {
    s_pCallbackInstance = this;
    m_pInterface->RunCallbacks();
  }
};

void
PrintUsageAndExit(int rc = 1)
{
  fflush(stderr);
  printf(
    R"usage(Usage:
    example_chat client SERVER_ADDR
)usage");
  fflush(stdout);
  exit(rc);
}

ChatClient* ChatClient::s_pCallbackInstance = nullptr;

int
main(int argc, const char* argv[])
{
  const uint16 DEFAULT_SERVER_PORT = 27015;
  printf("hello, client! \n");

  bool bServer = false;
  bool bClient = false;
  int nPort = DEFAULT_SERVER_PORT;
  SteamNetworkingIPAddr addrServer;
  addrServer.Clear();

  for (int i = 1; i < argc; ++i) {
    if (!bClient && !bServer) {
      if (!strcmp(argv[i], "client")) {
        bClient = true;
        continue;
      }
      if (!strcmp(argv[i], "server")) {
        bServer = true;
        continue;
      }
    }

    if (!strcmp(argv[i], "--port")) {
      ++i;
      if (i >= argc)
        PrintUsageAndExit();
      nPort = atoi(argv[i]);
      if (nPort <= 0 || nPort > 65535)
        FatalError("Invalid port %d", nPort);
      continue;
    }

    // Anything else, must be server address to connect to
    if (bClient && addrServer.IsIPv6AllZeros()) {
      if (!addrServer.ParseString(argv[i]))
        FatalError("Invalid server address '%s'", argv[i]);
      if (addrServer.m_port == 0)
        addrServer.m_port = DEFAULT_SERVER_PORT;
      continue;
    }

    PrintUsageAndExit();
  }

  if (bClient == bServer || (bClient && addrServer.IsIPv6AllZeros()))
    PrintUsageAndExit();

  // Create client and server sockets
  InitSteamDatagramConnectionSockets();
  LocalUserInput_Init();

  {
    ChatClient client;
    client.Run(addrServer);
  }

  ShutdownSteamDatagramConnectionSockets();

  // Ug, why is there no simple solution for portable, non-blocking console user input?
  // Just nuke the process
  // LocalUserInput_Kill();
  NukeProcess(0);

  return 0;
}