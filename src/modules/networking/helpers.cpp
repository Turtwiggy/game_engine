#include "helpers.hpp"

#include <assert.h>
#include <iostream>

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

void
start_server_or_quit(entt::registry& r, int port, void* callback)
{
  SINGLETON_ServerComponent server;

  // Select instance to use.  For now we'll always use the default.
  // But we could use SteamGameServerNetworkingSockets() on Steam.
  server.interface = SteamNetworkingSockets();

  // Start Listening
  SteamNetworkingIPAddr addr;
  addr.Clear();
  addr.m_port = port;

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, callback);

  server.socket = server.interface->CreateListenSocketIP(addr, 1, &opt);
  if (server.socket == k_HSteamListenSocket_Invalid) {
    std::cerr << "Failed to listen on port: " << port << std::endl;
    exit(0);
  }

  server.group = server.interface->CreatePollGroup();
  if (server.socket == k_HSteamNetPollGroup_Invalid) {
    std::cerr << "Failed to listen on port: " << std::endl;
    exit(0);
  }

  std::cout << "Server listening on port: " << port << std::endl;
  if (!r.ctx().contains<SINGLETON_ServerComponent>())
    r.ctx().emplace<SINGLETON_ServerComponent>(server);
}

void
start_client(entt::registry& r, const std::string& addr, void* callback)
{
  SINGLETON_ClientComponent client;
  client.interface = SteamNetworkingSockets();

  SteamNetworkingIPAddr serverAddr;
  serverAddr.Clear();
  if (serverAddr.ParseString(addr.c_str())) {
    // Start connecting
    char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
    serverAddr.ToString(szAddr, sizeof(szAddr), true);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, callback);
    auto conn = client.interface->ConnectByIPAddress(serverAddr, 1, &opt);
    if (conn == k_HSteamNetConnection_Invalid) {
      std::cerr << "(client) invalid server addr: " << szAddr << std::endl;
    } else {
      // successfully connected
      client.connection = conn;
      r.ctx().emplace<SINGLETON_ClientComponent>(client);
    }
  } else {
    std::cerr << "Failed to parse server's address for the client" << std::endl;
  }
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

void
send_string_to_client(ISteamNetworkingSockets* interface, HSteamNetConnection conn, const std::string& str)
{
  interface->SendMessageToConnection(
    conn, str.c_str(), (uint32)strlen(str.c_str()), k_nSteamNetworkingSend_Reliable, nullptr);
}

void
send_string_to_all_clients(ISteamNetworkingSockets* interface,
                           std::vector<HSteamNetConnection>& clients,
                           const char* str,
                           HSteamNetConnection except)
{
  for (auto& c : clients)
    send_string_to_client(interface, c, str);
}

void
server_receive_messages_on_poll_group(SINGLETON_ServerComponent& server, std::vector<std::string>& result)
{
  constexpr int max_messages = 32;
  ISteamNetworkingMessage* all_msgs[max_messages] = {};
  int num_msgs = server.interface->ReceiveMessagesOnPollGroup(server.group, all_msgs, max_messages);

  if (num_msgs < 0)
    std::cerr << "Error checking for messages" << std::endl;

  for (int i = 0; i < num_msgs; i++) {
    ISteamNetworkingMessage* msg = all_msgs[i];
    assert(msg);

    auto conn = std::find(server.clients.begin(), server.clients.end(), msg->m_conn);
    assert(conn != server.clients.end());

    std::string sCmd;
    sCmd.assign((const char*)msg->m_pData, msg->m_cbSize);
    msg->Release(); // We don't need this anymore.

    result.push_back(sCmd);
  }
}

void
client_receive_messages_on_connection(SINGLETON_ClientComponent& client, std::vector<std::string>& result)
{
  constexpr int max_messages = 32;
  ISteamNetworkingMessage* all_msgs[max_messages] = {};
  int num_msgs = client.interface->ReceiveMessagesOnConnection(client.connection, all_msgs, max_messages);

  if (num_msgs < 0)
    std::cerr << "(client) error checking for messages" << std::endl;

  {
    for (int i = 0; i < num_msgs; i++) {
      ISteamNetworkingMessage* msg = all_msgs[i];
      assert(msg);

      std::string data;
      data.assign((const char*)msg->m_pData, msg->m_cbSize);
      msg->Release(); // We don't need this anymore.

      result.push_back(data);
    }
  }
}

} // namespace game2d