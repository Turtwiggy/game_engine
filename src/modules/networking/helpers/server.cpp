#include "server.hpp"

#include "engine/app/application.hpp"
#include "engine/maths/grid.hpp"
#include "game/create_entities.hpp"
#include "game/game_tick.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/networking/components.hpp"

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
start_server_or_quit(entt::registry& r, int port)
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
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

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
game2d::server_receive_messages_on_poll_group(SINGLETON_ServerComponent& server, std::vector<ClientMessage>& result)
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
    result.push_back({ sCmd, msg->m_conn });

    msg->Release(); // We don't need this anymore.
  }
}

void
game2d::server_poll_connections(SINGLETON_ServerComponent& server)
{
  // PollConnectionStateChanges
  //
  server.interface->RunCallbacks();
  server.events.clear();

  // Process Callbacks
  //
  for (auto& info : conn_event) {
    //
    // Process event
    //
    switch (info.m_info.m_eState) {
      case k_ESteamNetworkingConnectionState_None: {
        // NOTE: We will get callbacks here when we destroy connections.
        // You can ignore these.
        break;
      }
      case k_ESteamNetworkingConnectionState_Connected: {
        // We will get a callback immediately after accepting the connection.
        // Since we are the server, we can ignore this, it's not news to us.
        break;
      }
      case k_ESteamNetworkingConnectionState_ClosedByPeer:
      case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        if (info.m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
          // Locate the client.  Note that it should have been found, because this
          // is the only codepath where we remove clients (except on shutdown),
          // and connection change callbacks are dispatched in queue order.
          auto client_it = std::find(server.clients.begin(), server.clients.end(), info.m_hConn);
          assert(client_it != server.clients.end());

          // Select appropriate log messages
          const char* pszDebugLogAction;
          if (info.m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            pszDebugLogAction = "problem detected locally";
          else
            pszDebugLogAction = "closed by peer";

          std::cout << "Connection: " << info.m_info.m_szConnectionDescription << " " << pszDebugLogAction << " "
                    << std::to_string(info.m_info.m_eEndReason) << " " << info.m_info.m_szEndDebug << std::endl;

          // Send a message so everybody else knows what happened
          // send_string_to_all_clients(temp);
          server.clients.erase(client_it);
          server.events.push_back(ServerEvents::CLIENT_DROPPED);
        } else {
          assert(info.m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
        }

        // Clean up the connection.  This is important!
        // The connection is "closed" in the network sense, but
        // it has not been destroyed.  We must close it on our end, too
        // to finish up.  The reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0's.
        server.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
        break;
      }
      case k_ESteamNetworkingConnectionState_Connecting: {
        // must be a new connection
        auto conn = std::find(server.clients.begin(), server.clients.end(), info.m_hConn);
        assert(conn == server.clients.end());

        std::cout << "Connection request from " << info.m_info.m_szConnectionDescription << std::endl;

        // Arbitrarily limit slots on server
        // until I'm better at understanding networking stuff
        bool slots_available = server.clients.size() <= server.max_clients;
        if (!slots_available) {
          std::cout << "Can't accept connection. (Server Full)" << std::endl;
          int reason = 1;
          server.interface->CloseConnection(info.m_hConn, reason, nullptr, false);
          break;
        }

        // A client is attempting to connect
        // Try to accept the connection.
        if (server.interface->AcceptConnection(info.m_hConn) != k_EResultOK) {
          // This could fail.  If the remote host tried to connect, but then
          // disconnected, the connection may already be half closed.  Just
          // destroy whatever we have on our side.
          server.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
          std::cout << "Can't accept connection.  (Already closed?)" << std::endl;
          break;
        }

        // Assign the poll group
        if (!server.interface->SetConnectionPollGroup(info.m_hConn, server.group)) {
          server.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
          std::cout << "Failed to set poll group?" << std::endl;
          break;
        }

        // send_string_to_client(server.interface, info.m_hConn, std::string("Welcome!"));

        // Add them to the client list
        server.clients.push_back(info.m_hConn);
        server.events.push_back(ServerEvents::CLIENT_JOINED);
        break;
      }
    }
  }
  conn_event.clear();
}

void
game2d::tick_server(entt::registry& r, uint64_t milliseconds_dt)
{
  SINGLETON_ServerComponent& server = r.ctx().at<SINGLETON_ServerComponent>();
  server_poll_connections(server);
  server.fixed_frame += 1;

  // PollIncomingMessages()
  std::vector<ClientMessage> client_messages;
  server_receive_messages_on_poll_group(server, client_messages);

  // ProcessClientMessage()
  std::cout << "(server) waiting for client to send ft: " << server.fixed_frame << std::endl;
  if (client_messages.size() == 0)
    return;
  // for (int i = 0; i < client_messages.size(); i++) {
  ClientMessage message = client_messages[0];

  // Assume all messages are input, so ProcessClientInput()
  //   SINGLETON_FixedUpdateInputHistory player_input_history;
  //   player_input_history = nlohmann::json::from_cbor(message.data);

  //   int difference = player_input_history.fixed_tick - server.fixed_frame;
  //   std::cout << "(server) client is " << difference << " ahead" << std::endl;
  //   std::cout << "(server) has " << player_input_history.history.size() << " to search" << std::endl;

  // #ifdef _DEBUG
  //   if (difference > 200) {
  //     std::cerr << "(server) client is 200 ticks ahead??" << std::endl;
  //   }
  // #endif

  //   // remove duds
  //   if (player_input_history.history.size() - difference > 0) {
  //     std::vector<std::vector<InputEvent>>::iterator it = player_input_history.history.end() - difference;
  //     while (it != player_input_history.history.end()) {

  //       const auto& inputs = it;

  //       // HACK: just set entity to the only player entity the server has.
  //       auto player_id = r.view<const PlayerComponent>().front();
  //       for (int i = 0; i < (*inputs).size(); i++) {
  //         (*inputs)[i].player = player_id;
  //       }

  //       simulate(r, *inputs, milliseconds_dt);
  //       ++it;
  //     }

  //     send_string_to_client(server.interface, message.conn, std::to_string(server.fixed_frame));
};

//
// Server -> Client
//

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

} // namespace game2d