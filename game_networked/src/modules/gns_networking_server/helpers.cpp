#include "helpers.hpp"

#include "app/application.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"

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
start_server_or_quit(entt::registry& r, uint16 port)
{
  std::cout << "starting server..." << std::endl;

  SINGLETON_ServerComponent server;

  // Select instance to use.  For now we'll always use the default.
  // But we could use SteamGameServerNetworkingSockets() on Steam.
  server.interface = SteamNetworkingSockets();

  // Start Listening
  SteamNetworkingIPAddr server_local_adr;
  server_local_adr.Clear();
  server_local_adr.m_port = port;

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback);

  server.socket = server.interface->CreateListenSocketIP(server_local_adr, 1, &opt);
  if (server.socket == k_HSteamListenSocket_Invalid) {
    std::cerr << "Failed to listen on port: " << port << std::endl;
    exit(0);
  }

  server.group = server.interface->CreatePollGroup();
  if (server.group == k_HSteamNetPollGroup_Invalid) {
    std::cerr << "Failed to listen on port: " << port << std::endl;
    exit(0);
  }

  std::cout << "Server listening on port: " << port << std::endl;
  r.emplace<SINGLETON_ServerComponent>(r.create(), server);
}

void
server_receive_messages_on_poll_group(SINGLETON_ServerComponent& server, std::vector<ClientMessage>& result)
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
server_poll_connections(SINGLETON_ServerComponent& server)
{
  // Process Callbacks
  for (auto& info : conn_event) {
    switch (info.m_info.m_eState) {
      case k_ESteamNetworkingConnectionState_None: {
        // NOTE: We will get callbacks here when we destroy connections.
        // You can ignore these.
        break;
      }
      case k_ESteamNetworkingConnectionState_Connected: {
        // We will get a callback immediately after accepting the connection.
        // Since we are the server, we can ignore this, it's not news to us.

        // send a welcome message because why not
        send_string_to_client(server.interface, info.m_hConn, std::string("Welcome!"));

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
                    << std::to_string(info.m_info.m_eEndReason) << " " << info.m_info.m_szEndDebug << "\n";

          // Send a message so everybody else knows what happened
          // send_string_to_all_clients(temp);
          server.clients.erase(client_it);
          // server.events.push_back(ServerEvents::CLIENT_DROPPED);
        } else
          assert(info.m_eOldState == k_ESteamNetworkingConnectionState_Connecting);

        // Clean up the connection. This is important!
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
          std::cout << "Cant accept connection. (Server Full)" << std::endl;
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
          std::cout << "Can't accept connection. (Already closed?)" << std::endl;
          break;
        }

        // Assign the poll group
        if (!server.interface->SetConnectionPollGroup(info.m_hConn, server.group)) {
          server.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
          std::cout << "Failed to set poll group?" << std::endl;
          break;
        }

        // Add them to the client list
        server.clients.push_back(info.m_hConn);
        break;
      }
    }
  }
  conn_event.clear();
}

void
tick_server(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& server = get_first_component<SINGLETON_ServerComponent>(r);

  // PollIncomingMessages()
  std::vector<ClientMessage> client_messages;
  server_receive_messages_on_poll_group(server, client_messages);

  // PollConnectionStateChanges();
  server.interface->RunCallbacks();
  server_poll_connections(server);

  // ProcessClientMessage()
  for (auto i = 0; i < client_messages.size(); i++) {
    ClientMessage message = client_messages[i];
    std::cout << "(server) from client " << message.conn << " recieved:" << message.data << std::endl;
    server.incoming_messages.push_back(message.data);
  }
};

void
shutdown(SINGLETON_ServerComponent& server)
{
  // 	// Close all the connections
  // Printf( "Closing connections...\n" );
  // for ( auto it: m_mapClients )
  // {
  // 	// Send them one more goodbye message.  Note that we also have the
  // 	// connection close reason as a place to send final data.  However,
  // 	// that's usually best left for more diagnostic/debug text not actual
  // 	// protocol strings.
  // 	SendStringToClient( it.first, "Server is shutting down.  Goodbye." );

  // 	// Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
  // 	// to flush this out and close gracefully.
  // 	m_pInterface->CloseConnection( it.first, 0, "Server Shutdown", true );
  // }
  // m_mapClients.clear();

  // m_pInterface->CloseListenSocket( m_hListenSock );
  // m_hListenSock = k_HSteamListenSocket_Invalid;

  // m_pInterface->DestroyPollGroup( m_hPollGroup );
  // m_hPollGroup = k_HSteamNetPollGroup_Invalid;
}

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
                           const std::vector<HSteamNetConnection>& clients,
                           const char* str)
{
  for (const auto& c : clients)
    send_string_to_client(interface, c, str);
}

void
send_string_to_all_clients(const SINGLETON_ServerComponent& server, const std::string& s)
{
  send_string_to_all_clients(server.interface, server.clients, s.c_str());
}

} // namespace game2d