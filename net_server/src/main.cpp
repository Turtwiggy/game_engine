
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

// engine
#include "engine/networking/test.hpp"

#define STEAMNETWORKINGSOCKETS_OPENSOURCE
#include "engine/networking/net_common.hpp"
using namespace net_common;

class ChatServer
{
public:
  void Run(uint16 nPort)
  {
    // Select instance to use.  For now we'll always use the default.
    // But we could use SteamGameServerNetworkingSockets() on Steam.
    m_pInterface = SteamNetworkingSockets();

    // Start listening
    SteamNetworkingIPAddr serverLocalAddr;
    serverLocalAddr.Clear();
    serverLocalAddr.m_port = nPort;
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);
    m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
    if (m_hListenSock == k_HSteamListenSocket_Invalid)
      FatalError("Failed to listen on port %d", nPort);
    m_hPollGroup = m_pInterface->CreatePollGroup();
    if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
      FatalError("Failed to listen on port %d", nPort);
    Printf("Server listening on port %d\n", nPort);

    while (!g_bQuit) {
      PollIncomingMessages();
      PollConnectionStateChanges();
      PollLocalUserInput();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Close all the connections
    Printf("Closing connections...\n");
    for (auto it : m_mapClients) {
      // Send them one more goodbye message.  Note that we also have the
      // connection close reason as a place to send final data.  However,
      // that's usually best left for more diagnostic/debug text not actual
      // protocol strings.
      SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

      // Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
      // to flush this out and close gracefully.
      m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
    }
    m_mapClients.clear();

    m_pInterface->CloseListenSocket(m_hListenSock);
    m_hListenSock = k_HSteamListenSocket_Invalid;

    m_pInterface->DestroyPollGroup(m_hPollGroup);
    m_hPollGroup = k_HSteamNetPollGroup_Invalid;
  }

private:
  HSteamListenSocket m_hListenSock;
  HSteamNetPollGroup m_hPollGroup;
  ISteamNetworkingSockets* m_pInterface;

  struct Client_t
  {
    std::string m_sNick;
  };

  std::map<HSteamNetConnection, Client_t> m_mapClients;

  void SendStringToClient(HSteamNetConnection conn, const char* str)
  {
    m_pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
  }

  void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid)
  {
    for (auto& c : m_mapClients) {
      if (c.first != except)
        SendStringToClient(c.first, str);
    }
  }

  void PollIncomingMessages()
  {
    char temp[1024];

    while (!g_bQuit) {
      ISteamNetworkingMessage* pIncomingMsg = nullptr;
      int numMsgs = m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, &pIncomingMsg, 1);
      if (numMsgs == 0)
        break;
      if (numMsgs < 0)
        FatalError("Error checking for messages");
      assert(numMsgs == 1 && pIncomingMsg);
      auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
      assert(itClient != m_mapClients.end());

      // '\0'-terminate it to make it easier to parse
      std::string sCmd;
      sCmd.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
      const char* cmd = sCmd.c_str();

      // We don't need this anymore.
      pIncomingMsg->Release();

      // Check for known commands.  None of this example code is secure or robust.
      // Don't write a real server like this, please.

      if (strncmp(cmd, "/nick", 5) == 0) {
        const char* nick = cmd + 5;
        while (isspace(*nick))
          ++nick;

        // Let everybody else know they changed their name
        sprintf_s(temp, "%s shall henceforth be known as %s", itClient->second.m_sNick.c_str(), nick);
        SendStringToAllClients(temp, itClient->first);

        // Respond to client
        sprintf_s(temp, "Ye shall henceforth be known as %s", nick);
        SendStringToClient(itClient->first, temp);

        // Actually change their name
        SetClientNick(itClient->first, nick);
        continue;
      }

      // Assume it's just a ordinary chat message, dispatch to everybody else
      sprintf_s(temp, "%s: %s", itClient->second.m_sNick.c_str(), cmd);
      SendStringToAllClients(temp, itClient->first);
    }
  }

  void PollLocalUserInput()
  {
    std::string cmd;
    while (!g_bQuit && LocalUserInput_GetNext(cmd)) {
      if (strcmp(cmd.c_str(), "/quit") == 0) {
        g_bQuit = true;
        Printf("Shutting down server");
        break;
      }

      if (strcmp(cmd.c_str(), "/help") == 0) {
        Printf("Command: /quit");
        Printf("Command: /help");
        break;
      }

      if (strcmp(cmd.c_str(), "/potato") == 0) {
        Printf("Why'd you type potato on the server?");
        break;
      }

      // That's the only command we support
      Printf("For commands, type: '/help'. '/quit'");
    }
  }

  void SetClientNick(HSteamNetConnection hConn, const char* nick)
  {

    // Remember their nick
    m_mapClients[hConn].m_sNick = nick;

    // Set the connection name, too, which is useful for debugging
    m_pInterface->SetConnectionName(hConn, nick);
  }

  void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
  {
    char temp[1024];

    // What's the state of the connection?
    switch (pInfo->m_info.m_eState) {
      case k_ESteamNetworkingConnectionState_None:
        // NOTE: We will get callbacks here when we destroy connections.  You can ignore
        // these.
        break;

      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        // Ignore if they were not previously connected.  (If they disconnected
        // before we accepted the connection.)
        if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {

          // Locate the client.  Note that it should have been found, because this
          // is the only codepath where we remove clients (except on shutdown),
          // and connection change callbacks are dispatched in queue order.
          auto itClient = m_mapClients.find(pInfo->m_hConn);
          assert(itClient != m_mapClients.end());

          // Select appropriate log messages
          const char* pszDebugLogAction;
          if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
            pszDebugLogAction = "problem detected locally";
            sprintf_s(temp,
                      "Alas, %s hath fallen into shadow.  (%s)",
                      itClient->second.m_sNick.c_str(),
                      pInfo->m_info.m_szEndDebug);
          } else {
            // Note that here we could check the reason code to see if
            // it was a "usual" connection or an "unusual" one.
            pszDebugLogAction = "closed by peer";
            sprintf_s(temp, "%s hath departed", itClient->second.m_sNick.c_str());
          }

          // Spew something to our own log.  Note that because we put their nick
          // as the connection description, it will show up, along with their
          // transport-specific data (e.g. their IP address)
          Printf("Connection %s %s, reason %d: %s\n",
                 pInfo->m_info.m_szConnectionDescription,
                 pszDebugLogAction,
                 pInfo->m_info.m_eEndReason,
                 pInfo->m_info.m_szEndDebug);

          m_mapClients.erase(itClient);

          // Send a message so everybody else knows what happened
          SendStringToAllClients(temp);
        } else {
          assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
        }

        // Clean up the connection.  This is important!
        // The connection is "closed" in the network sense, but
        // it has not been destroyed.  We must close it on our end, too
        // to finish up.  The reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0's.
        m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        break;
      }

      case k_ESteamNetworkingConnectionState_Connecting: {
        // This must be a new connection
        assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

        Printf("Connection request from %s", pInfo->m_info.m_szConnectionDescription);

        // A client is attempting to connect
        // Try to accept the connection.
        if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
          // This could fail.  If the remote host tried to connect, but then
          // disconnected, the connection may already be half closed.  Just
          // destroy whatever we have on our side.
          m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
          Printf("Can't accept connection.  (It was already closed?)");
          break;
        }

        // Assign the poll group
        if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup)) {
          m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
          Printf("Failed to set poll group?");
          break;
        }

        // Generate a random nick.  A random temporary nick
        // is really dumb and not how you would write a real chat server.
        // You would want them to have some sort of signon message,
        // and you would keep their client in a state of limbo (connected,
        // but not logged on) until them.  I'm trying to keep this example
        // code really simple.
        char nick[64];
        sprintf_s(nick, "BraveWarrior%d", 10000 + (rand() % 100000));

        // Send them a welcome message
        sprintf_s(temp,
                  "Welcome, stranger.  Thou art known to us for now as '%s'; upon thine "
                  "command '/nick' we shall know thee otherwise.",
                  nick);
        SendStringToClient(pInfo->m_hConn, temp);

        // Also send them a list of everybody who is already connected
        if (m_mapClients.empty()) {
          SendStringToClient(pInfo->m_hConn, "Thou art utterly alone.");
        } else {
          sprintf_s(temp, "%d companions greet you:", (int)m_mapClients.size());
          for (auto& c : m_mapClients)
            SendStringToClient(pInfo->m_hConn, c.second.m_sNick.c_str());
        }

        // Let everybody else know who they are for now
        sprintf_s(temp,
                  "Hark!  A stranger hath joined this merry host.  For now we shall call "
                  "them '%s'",
                  nick);
        SendStringToAllClients(temp, pInfo->m_hConn);

        // Add them to the client list, using std::map wacky syntax
        m_mapClients[pInfo->m_hConn];
        SetClientNick(pInfo->m_hConn, nick);
        break;
      }

      case k_ESteamNetworkingConnectionState_Connected:
        // We will get a callback immediately after accepting the connection.
        // Since we are the server, we can ignore this, it's not news to us.
        break;

      default:
        // Silences -Wswitch
        break;
    }
  }

  static ChatServer* s_pCallbackInstance;
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
    example_chat server [--port PORT]
)usage");
  fflush(stdout);
  exit(rc);
}

ChatServer* ChatServer::s_pCallbackInstance = nullptr;

int
main(int argc, const char* argv[])
{
  const uint16 DEFAULT_SERVER_PORT = 27020;
  int nPort = DEFAULT_SERVER_PORT;
  SteamNetworkingIPAddr addrServer;
  addrServer.Clear();

  bool bServer = false;
  bool bClient = false;

  printf("starting server on port: %i \n", DEFAULT_SERVER_PORT);

  for (int i = 1; i < argc; ++i) {
    printf("arg: %s \n", argv[i]);

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
    ChatServer server;
    server.Run((uint16)nPort);
  }

  ShutdownSteamDatagramConnectionSockets();
  NukeProcess(0);

  printf("exiting server... \n");
  return 0;
}