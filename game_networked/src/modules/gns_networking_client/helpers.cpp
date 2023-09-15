#include "helpers.hpp"

#include "app/application.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "maths/grid.hpp"

#include <nlohmann/json.hpp>

#include <iostream>

namespace game2d {

static std::vector<SteamNetConnectionStatusChangedCallback_t> conn_event;
static void
SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
  conn_event.push_back(*pInfo);
};

void
start_client(entt::registry& r, const std::string& addr, uint16 port)
{
  SINGLETON_ClientComponent client;
  client.interface = SteamNetworkingSockets();

  SteamNetworkingIPAddr serverAddr;
  serverAddr.Clear();
  serverAddr.m_port = port;

  if (!serverAddr.ParseString(addr.c_str())) {
    std::cerr << "Invalid server address: " << addr;
    return;
  }

  // Start connecting
  char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
  serverAddr.ToString(szAddr, sizeof(szAddr), true);
  std::cout << "connecting to server at: " << addr << std::endl;

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
  client.connection = client.interface->ConnectByIPAddress(serverAddr, 1, &opt);

  if (client.connection == k_HSteamNetConnection_Invalid) {
    std::cerr << "(client) failed to create connection" << std::endl;
    return;
  }
  r.emplace<SINGLETON_ClientComponent>(r.create(), client);
}

void
client_receive_messages_on_connection(SINGLETON_ClientComponent& client, std::vector<std::string>& result)
{
  constexpr int max_messages = 32;
  ISteamNetworkingMessage* all_msgs[max_messages] = {};
  int num_msgs = client.interface->ReceiveMessagesOnConnection(client.connection, all_msgs, max_messages);

  if (num_msgs < 0)
    std::cerr << "(client) error checking for messages" << std::endl;
  for (int i = 0; i < num_msgs; i++) {
    ISteamNetworkingMessage* msg = all_msgs[i];
    assert(msg);

    std::string data;
    data.assign((const char*)msg->m_pData, msg->m_cbSize);
    msg->Release(); // We don't need this anymore.

    result.push_back(data);
  }
}

void
client_poll_connections(SINGLETON_ClientComponent& client)
{
  // PollConnectionStateChanges
  //
  client.interface->RunCallbacks();

  // Process Callback Results
  //
  for (auto& info : conn_event) {
    // What's the state of the connection?
    switch (info.m_info.m_eState) {
      case k_ESteamNetworkingConnectionState_None:
        // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
        break;
      case k_ESteamNetworkingConnectionState_Connecting:
        std::cout << "Connecting to server..." << std::endl;
        break;
      case k_ESteamNetworkingConnectionState_Connected:
        std::cout << "Connected to server OK!" << std::endl;
        break;

      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {

        // Print an appropriate message
        if (info.m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
          // Note: we could distinguish between a timeout, a rejected connection,
          // or some other transport problem.
          std::cout << "We sought the remote host, yet our efforts were met with defeat." << info.m_info.m_szEndDebug
                    << std::endl;

        } else if (info.m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
          std::cout << "Alas, troubles beset us; we have lost contact with the host. " << info.m_info.m_szEndDebug
                    << std::endl;
        } else {
          // NOTE: We could check the reason code for a normal disconnection
          std::cout << "The host hath bidden us farewell. " << info.m_info.m_szEndDebug << std::endl;
        }

        // Clean up the connection.  This is important!
        // The connection is "closed" in the network sense, but
        // it has not been destroyed.  We must close it on our end, too
        // to finish up.  The reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0's.
        client.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
        client.connection = k_HSteamNetConnection_Invalid;
        break;
      }
      default:
        break;
    }
  }
  conn_event.clear();
}

void
tick_client(entt::registry& r, uint64_t milliseconds_dt)
{
  // PollConnectionStateChanges()
  auto& client = get_first_component<SINGLETON_ClientComponent>(r);
  client_poll_connections(client);

  // PollIncomingMessages()
  std::vector<std::string> server_messages;
  client_receive_messages_on_connection(client, server_messages);

  // Debug Messages
  client.messages.clear();
  for (int i = 0; i < server_messages.size(); i++) {
    std::string message = server_messages[i];
    std::cout << "(client) received: " << message << "\n";
    client.messages.push_back(message);
  }

  // PollLocalUserInput()
  // TODO: https://gafferongames.com/post/deterministic_lockstep/
  // TODO: https://gafferongames.com/post/what_every_programmer_needs_to_know_about_game_networking/
}

void
send_message_to_server(const SINGLETON_ClientComponent& client, const std::string& packet)
{
  // auto data = nlohmann::json::to_cbor(message);
  // std::string packet;
  // packet.assign(data.begin(), data.end());
  const int protocol = k_nSteamNetworkingSend_Unreliable;
  const int max_size = k_cbMaxSteamNetworkingSocketsMessageSizeSend;

  if (packet.size() >= max_size)
    std::cerr << "packet wanting to send is too large, sending anyway..." << std::endl;

  client.interface->SendMessageToConnection(client.connection, packet.c_str(), (uint32_t)packet.size(), protocol, nullptr);
}

} // namespace game2d