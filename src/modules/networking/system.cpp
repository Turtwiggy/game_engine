#include "system.hpp"

#include "engine/maths/grid.hpp"
#include "game/create_entities.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/networking/components.hpp"
#include "modules/networking/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_networking/components.hpp"

#include <assert.h>
#include <imgui.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif
#include <nlohmann/json.hpp>

namespace game2d {

static std::vector<SteamNetConnectionStatusChangedCallback_t> conn_event;
static void
SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
  conn_event.push_back(*pInfo);
};

void
update_client_poll_connections(SINGLETON_ClientComponent& client)
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
update_server_poll_connections(SINGLETON_ServerComponent& server)
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

        send_string_to_client(server.interface, info.m_hConn, std::string("Welcome!"));

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
server_tick(entt::registry& r)
{
  SINGLETON_ServerComponent& server = r.ctx().at<SINGLETON_ServerComponent>();
  update_server_poll_connections(server);

  server.process_fixed_frame += 1;
  if (server.events.size() > 0) {
    // someone joined or left
    std::cout << "someone joined or left..." << std::endl;
  }

  // PollIncomingMessages
  ISteamNetworkingMessage* msg = nullptr;
  int msgs = server.interface->ReceiveMessagesOnPollGroup(server.group, &msg, 1);

  if (msgs < 0)
    std::cerr << "Error checking for messages" << std::endl;

  if (msgs > 0) {
    assert(msgs == 1 && msg);

    auto conn = std::find(server.clients.begin(), server.clients.end(), msg->m_conn);
    assert(conn != server.clients.end());

    // '\0'-terminate it to make it easier to parse
    std::string sCmd;
    sCmd.assign((const char*)msg->m_pData, msg->m_cbSize);

    // We don't need this anymore.
    msg->Release();

    // Check for known commands. WARNING: un-sanitized input from client
    // std::cout << "(server) data from client: " << sCmd << std::endl;

    // Input history for each player?
    // SINGLETON_FixedUpdateInputHistory player_input_history;
    // player_input_history = nlohmann::json::from_cbor(sCmd);

    // WARNING: this game logic doesn't belong here
    // if (strcmp(sCmd.c_str(), ",spawn") == 0) {
    //   std::cout << "(server) client requested to spawn a player" << std::endl;
    //   auto player = create_player(r);
    //   auto& player_transform = r.get<TransformComponent>(player);
    //   player_transform.position.x = 600;
    //   player_transform.position.y = 400;
    //   auto& player_speed = r.get<PlayerComponent>(player);
    //   player_speed.speed = 250.0f;
    //   send_string_to_client(server.interface, *conn, "Server recieved spawn request");
    // }
  };
};

void
client_tick(entt::registry& r)
{
  SINGLETON_ClientComponent& client = r.ctx().at<SINGLETON_ClientComponent>();
  update_client_poll_connections(client);

  // ... do client things ...

  // PollIncomingMessages()
  ISteamNetworkingMessage* pIncomingMsg = nullptr;
  int numMsgs = client.interface->ReceiveMessagesOnConnection(client.connection, &pIncomingMsg, 1);
  if (numMsgs < 0)
    std::cerr << "(client) error checking for messages" << std::endl;
  if (numMsgs > 0) {
    std::string data;
    data.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
    std::cout << "(client) server sent: " << data << std::endl;
    pIncomingMsg->Release(); // We don't need this anymore.
  }
  if (numMsgs > 1) {
    std::cout << "pause! how to handle more than one message" << std::endl;
  }

  // PollLocalUserInput()
  // HACK: the below is just to get some input sending to the server
  // TODO: try a different keyboard
  // Send inputs and input history every fixed tick.
  // TODO: https://gafferongames.com/post/deterministic_lockstep/
  // TODO: https://gafferongames.com/post/what_every_programmer_needs_to_know_about_game_networking/

  // the simulation can only simulate frame n when it has hte input for that frame
  // if it doens't have the input, it has to wait.

  // auto& inputs = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();
  // auto json = inputs;
  // auto cbor = nlohmann::json::to_cbor(json);

  std::string packet = "honk";
  // packet.assign(cbor.begin(), cbor.end());

  const int protocol = k_nSteamNetworkingSend_Unreliable;
  const int max_size = k_cbMaxSteamNetworkingSocketsMessageSizeSend;
  if (packet.size() >= max_size)
    std::cerr << "packet wanting to send is too large, sending anyway..." << std::endl;
  std::cout << "packet size: " << packet.size() << std::endl;
  client.interface->SendMessageToConnection(
    client.connection, packet.c_str(), (uint32_t)packet.size(), protocol, nullptr);
};

} // namespace game2d

void
game2d::init_networking_system(entt::registry& r)
{
  init_steam_datagram_connection_sockets();
};

void
game2d::update_networking_system(entt::registry& r)
{
  {
    auto& ui = r.ctx().at<SINGLETON_NetworkingUIComponent>();
    if (ui.start_server) {
      int port = 27020;
      start_server_or_quit(r, port, (void*)SteamNetConnectionStatusChangedCallback);
      ui.start_server = false;
    }

    if (ui.start_client) {
      static std::string localhost = "127.0.0.1:27020";
      start_client(r, localhost, (void*)SteamNetConnectionStatusChangedCallback);
      ui.start_client = false;
    }

    if (ui.close_networking) {
      close_networking();
      ui.close_networking = false;
    }
  }

  if (r.ctx().contains<SINGLETON_ServerComponent>())
    server_tick(r);

  if (r.ctx().contains<SINGLETON_ClientComponent>())
    client_tick(r);
};