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
start_client(entt::registry& r, const std::string& addr)
{
  auto& client = get_first_component<SINGLETON_ClientComponent>(r);
  client.interface = SteamNetworkingSockets();

  SteamNetworkingIPAddr serverAddr;
  serverAddr.Clear();
  if (serverAddr.ParseString(addr.c_str())) {
    // Start connecting
    char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
    serverAddr.ToString(szAddr, sizeof(szAddr), true);

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);
    auto conn = client.interface->ConnectByIPAddress(serverAddr, 1, &opt);
    if (conn == k_HSteamNetConnection_Invalid) {
      std::cerr << "(client) invalid server addr: " << szAddr << "\n";
    } else {
      // successfully connected
      client.connection = conn;
    }
  } else
    std::cerr << "Failed to parse server's address for the client\n";
}

void
game2d::client_receive_messages_on_connection(SINGLETON_ClientComponent& client, std::vector<std::string>& result)
{
  constexpr int max_messages = 32;
  ISteamNetworkingMessage* all_msgs[max_messages] = {};
  int num_msgs = client.interface->ReceiveMessagesOnConnection(client.connection, all_msgs, max_messages);

  if (num_msgs < 0)
    std::cerr << "(client) error checking for messages"
              << "\n";

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

void
game2d::client_poll_connections(SINGLETON_ClientComponent& client)
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
        std::cout << "Connecting to server..."
                  << "\n";
        break;
      case k_ESteamNetworkingConnectionState_Connected:
        std::cout << "Connected to server OK!"
                  << "\n";
        break;

      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        // Print an appropriate message
        if (info.m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
          // Note: we could distinguish between a timeout, a rejected connection,
          // or some other transport problem.
          std::cout << "We sought the remote host, yet our efforts were met with defeat." << info.m_info.m_szEndDebug
                    << "\n";
        } else if (info.m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
          std::cout << "Alas, troubles beset us; we have lost contact with the host. " << info.m_info.m_szEndDebug << "\n";
        } else {
          // NOTE: We could check the reason code for a normal disconnection
          std::cout << "The host hath bidden us farewell. " << info.m_info.m_szEndDebug << "\n";
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
game2d::tick_client(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& client = get_first_component<SINGLETON_ClientComponent>(r);
  // auto& fixed_inputs = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  client_poll_connections(client);
  client.fixed_frame += 1;

  // PollIncomingMessages()
  std::vector<std::string> server_messages;
  client_receive_messages_on_connection(client, server_messages);

  // HACK: move all unprocessed inputs from Update() to FixedUpdate()?
  // {
  //   auto& input = r.ctx().at<SINGLETON_InputComponent>();
  //   auto& fixed_input = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();
  //   fixed_input.history.push_back(std::move(input.unprocessed_update_inputs));

  //   // As a client, just simulate immediately
  //   simulate(r, fixed_input.history.back(), milliseconds_dt);
  //   client.simulation_frame += 1;
  // }

  // for (int i = 0; i < server_messages.size(); i++) {
  if (server_messages.size() > 0) {
    std::string message = server_messages[0];
    std::cout << "(client) received: " << message << "\n";

    // HACK: assume the message was an int containing an ack frame the server received...
    // int server_tick = std::stoi(message);
    // int client_tick = fixed_inputs.history.back();
    // fixed_inputs.fixed_tick_since_ack = fixed_inputs.fixed_tick - server_tick;

    // std::cout << "(client) tick: " << fixed_inputs.fixed_tick << " , ticks_ahead: " <<
    // fixed_inputs.fixed_tick_since_ack
    //           << "\n";

    // discard any unneeded inputs
    // while (fixed_inputs.history.size() < fixed_inputs.fixed_tick_since_ack)
    //   fixed_inputs.history.erase(fixed_inputs.history.begin());

    //     int excess = fixed_inputs.history.size() - fixed_inputs.fixed_tick_since_ack;
    //     if (excess > 0)
    //       fixed_inputs.history.erase(fixed_inputs.history.begin(), fixed_inputs.history.begin() + excess);
    // #ifdef _DEBUG
    //     assert(fixed_inputs.history.size() == fixed_inputs.fixed_tick_since_ack);
    // #endif
    // std::cout << "(client) inputs size: " << fixed_inputs.history.size() << "\n";
  }

  // ... do client things ...

  // PollLocalUserInput()
  // TODO: https://gafferongames.com/post/deterministic_lockstep/
  // TODO: https://gafferongames.com/post/what_every_programmer_needs_to_know_about_game_networking/

  //   {
  //     auto json = fixed_inputs;
  //     auto data = nlohmann::json::to_cbor(json);
  //     std::string packet;
  //     packet.assign(data.begin(), data.end());

  //     const int protocol = k_nSteamNetworkingSend_Unreliable;
  //     const int max_size = k_cbMaxSteamNetworkingSocketsMessageSizeSend;

  //     if (packet.size() >= max_size)
  //       std::cerr << "packet wanting to send is too large, sending anyway..." << "\n";

  //     client.interface->SendMessageToConnection(
  //       client.connection, packet.c_str(), (uint32_t)packet.size(), protocol, nullptr);

  //     fixed_inputs.fixed_tick += 1;
  //     fixed_inputs.fixed_tick_since_ack += 1;

  // #ifdef _DEBUG
  //     if (fixed_inputs.fixed_tick_since_ack > 200) {
  //       std::cerr << "(client) server is 200 ticks behind??" << "\n";
  //     }
  // #endif
  //   }
}

} // namespace game2d