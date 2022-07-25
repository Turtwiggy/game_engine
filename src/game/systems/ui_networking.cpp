#include "ui_place_entity.hpp"

#include "engine/maths/grid.hpp"
#include "game/create_entities.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/networking/components.hpp"
#include "modules/networking/helpers.hpp"
#include "modules/renderer/components.hpp"

#include <imgui.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <assert.h>
#include <iostream>
#include <map>
#include <string>

namespace game2d {

void
init_networking_system(entt::registry& r)
{
  InitSteamDatagramConnectionSockets();

  // SteamNetworkingIpAddr server_address;
  // server_address.clear();
}

static std::vector<SteamNetConnectionStatusChangedCallback_t> conn_event;
static void
SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
  conn_event.push_back(*pInfo);
};

void
update_ui_networking_system(entt::registry& r)
{
#ifdef _DEBUG
  // bool show_imgui_demo_window = false;
  // ImGui::ShowDemoWindow(&show_imgui_demo_window);
  // less than X-fps?! what is this?!
  if (ImGui::GetIO().Framerate <= 45 && ImGui::GetFrameCount() > 120)
    std::cout << "(profiler) fps drop?!" << std::endl;
#endif

  ImGui::Begin("Networking...");

  // static char buf[64] = { "0.0.0.0" };
  // if (ImGui::InputText("ServerAddr", &buf[0], IM_ARRAYSIZE(buf))) {
  //   //
  // }

  if (ImGui::Button("Begin Server")) {
    // ChatServer server;
    // server.Run((uint16)nPort);

    SINGLETON_ServerComponent server;
    // Select instance to use.  For now we'll always use the default.
    // But we could use SteamGameServerNetworkingSockets() on Steam.
    server.interface = SteamNetworkingSockets();

    // Start Listening
    SteamNetworkingIPAddr addr;
    addr.Clear();
    addr.m_port = server.port;

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void*)SteamNetConnectionStatusChangedCallback);

    server.socket = server.interface->CreateListenSocketIP(addr, 1, &opt);
    if (server.socket == k_HSteamListenSocket_Invalid) {
      std::cerr << "Failed to listen on port: " << server.port << std::endl;
      exit(0);
    }

    server.group = server.interface->CreatePollGroup();
    if (server.socket == k_HSteamNetPollGroup_Invalid) {
      std::cerr << "Failed to listen on port: " << std::endl;
      exit(0);
    }

    std::cout << "Server listening on port: " << server.port << std::endl;
    if (!r.ctx().contains<SINGLETON_ServerComponent>()) {
      r.ctx().emplace<SINGLETON_ServerComponent>(server);
    }
    // else {
    //   r.ctx().at<SINGLETON_ServerComponent>() = SINGLETON_ServerComponent();
    // }
  };

  if (ImGui::Button("Begin Client")) {

    SINGLETON_ClientComponent client;
    client.interface = SteamNetworkingSockets();
    static std::string localhost = "127.0.0.1:" + std::to_string(client.port);

    SteamNetworkingIPAddr serverAddr;
    serverAddr.Clear();
    if (serverAddr.ParseString(localhost.c_str())) {
      // Start connecting
      char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
      serverAddr.ToString(szAddr, sizeof(szAddr), true);

      SteamNetworkingConfigValue_t opt;
      opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                 (void*)SteamNetConnectionStatusChangedCallback);
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

  //   if (ImGui::Button("(todo) close?")) {
  //     std::cout << "TODO (close connections)" << std::endl;
  // #ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  //     GameNetworkingSockets_Kill();
  // #else
  //     SteamDatagramClient_Kill();
  // #endif
  //   }

  ImGui::End();

  //
  // SERVER
  //
  if (r.ctx().contains<SINGLETON_ServerComponent>()) {
    SINGLETON_ServerComponent& server = r.ctx().at<SINGLETON_ServerComponent>();

    // PollIncomingMessages
    //
    ISteamNetworkingMessage* msg = nullptr;
    int msgs = server.interface->ReceiveMessagesOnPollGroup(server.group, &msg, 1);

    if (msgs < 0)
      std::cerr << "Error checking for messages" << std::endl;

    if (msgs > 0) {
      assert(msgs == 1 && msg);
      auto cli = server.clients.find(msg->m_conn);
      assert(cli != server.clients.end());

      // '\0'-terminate it to make it easier to parse
      std::string sCmd;
      sCmd.assign((const char*)msg->m_pData, msg->m_cbSize);
      const char* cmd = sCmd.c_str();

      // We don't need this anymore.
      msg->Release();

      // Check for known commands.  None of this example code is secure or robust.
      // Don't write a real server like this, please.
      std::cout << "(server) message from client: " << cmd << std::endl;
    }

    // PollConnectionStateChanges
    //
    server.interface->RunCallbacks();

    // Process Callbacks
    //
    for (auto& info : conn_event) {
      //
      // Process event
      //
      switch (info.m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
          // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
          break;
        case k_ESteamNetworkingConnectionState_Connected:
          // We will get a callback immediately after accepting the connection.
          // Since we are the server, we can ignore this, it's not news to us.
          break;
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
          // Locate the client.  Note that it should have been found, because this
          // is the only codepath where we remove clients (except on shutdown),
          // and connection change callbacks are dispatched in queue order.
          auto itClient = server.clients.find(info.m_hConn);
          assert(itClient != server.clients.end());

          // Select appropriate log messages
          const char* pszDebugLogAction;

          if (info.m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
            pszDebugLogAction = "problem detected locally";
            // std::cout << "Alas, there's a " << pszDebugLogAction << " " << info.m_info.m_szEndDebug << std::endl;
          } else {
            // Note that here we could check the reason code to see if
            // it was a "usual" connection or an "unusual" one.
            pszDebugLogAction = "closed by peer";
            // sprintf(temp, "%s hath departed", itClient->second.m_sNick.c_str());
          }

          std::cout << "Connection: " << info.m_info.m_szConnectionDescription << " " << pszDebugLogAction << " "
                    << std::to_string(info.m_info.m_eEndReason) << " " << info.m_info.m_szEndDebug << std::endl;

          // Send a message so everybody else knows what happened
          // SendStringToAllClients(temp);
          break;

        case k_ESteamNetworkingConnectionState_Connecting:
          // must be a new connection
          assert(server.clients.find(info.m_hConn) == server.clients.end());
          std::cout << "Connection request from " << info.m_info.m_szConnectionDescription << std::endl;

          // A client is attempting to connect
          // Try to accept the connection.
          if (server.interface->AcceptConnection(info.m_hConn) != k_EResultOK) {
            // This could fail.  If the remote host tried to connect, but then
            // disconnected, the connection may already be half closed.  Just
            // destroy whatever we have on our side.
            server.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
            std::cout << "Can't accept connection.  (It was already closed?)" << std::endl;
            break;
          }

          // Assign the poll group
          if (!server.interface->SetConnectionPollGroup(info.m_hConn, server.group)) {
            server.interface->CloseConnection(info.m_hConn, 0, nullptr, false);
            std::cout << "Failed to set poll group?" << std::endl;
            break;
          }

          SendStringToClient(server.interface, info.m_hConn, "Welcome!");
          break;
      }
    }
    conn_event.clear();
  }

  //
  // CLIENT
  //
  if (r.ctx().contains<SINGLETON_ClientComponent>()) {
    SINGLETON_ClientComponent& client = r.ctx().at<SINGLETON_ClientComponent>();

    // do some fun client things
    // PollIncomingMessages();
    //
    ISteamNetworkingMessage* pIncomingMsg = nullptr;
    int numMsgs = client.interface->ReceiveMessagesOnConnection(client.connection, &pIncomingMsg, 1);
    // if (numMsgs < 0)
    //   FatalError("Error checking for messages");
    if (numMsgs > 0) {
      // Just echo anything we get from the server
      std::cout << pIncomingMsg->m_pData << std::endl;

      // We don't need this anymore.
      pIncomingMsg->Release();
    }

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
}

} // namespace game2d