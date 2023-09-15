#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/gns_networking_client/components.hpp"
#include "modules/gns_networking_client/helpers.hpp"
#include "modules/gns_networking_server/components.hpp"
#include "modules/gns_networking_server/helpers.hpp"
#include "modules/gns_ui_networking/components.hpp"

#include <imgui.h>
#include <steam/isteamnetworkingutils.h>

namespace game2d {

void
update_ui_networking_system(entt::registry& r)
{
  auto& ui = get_first_component<SINGLETON_NetworkingUIComponent>(r);
  const auto server_entity = get_first<SINGLETON_ServerComponent>(r);
  const auto client_entity = get_first<SINGLETON_ClientComponent>(r);

  ImGui::Begin("Networking");

  if (client_entity == entt::null && server_entity == entt::null) {
    if (ImGui::Button("Start Server"))
      ui.start_server = true;

    if (ImGui::Button("Start Client"))
      ui.start_client = true;

    static char host[128] = "127.0.0.1:27020";
    if (ImGui::InputText("server host", host, IM_ARRAYSIZE(host)))
      ui.host = std::string(host);
  }

  if (client_entity != entt::null || server_entity != entt::null) {
    if (ImGui::Button("Stop Networking"))
      ui.close_networking = true;
  }

  if (ImGui::Button("Start Offline"))
    ui.start_offline = true;

  if (server_entity != entt::null) {
    auto& server = r.get<SINGLETON_ServerComponent>(server_entity);
    ImGui::Text("You are a server");
    ImGui::Text("You have %i clients", static_cast<int>(server.clients.size()));

    static char str1[128] = "Hello, world!";
    ImGui::InputText("input server", str1, IM_ARRAYSIZE(str1));
    if (ImGui::Button("Send to Clients"))
      send_string_to_all_clients(server, std::string(str1));

    //
    // View all incoming messages
    //
    ImGui::Separator();
    static std::vector<std::string> server_messages;
    server_messages.insert(server_messages.end(), server.incoming_messages.begin(), server.incoming_messages.end());
    // hack: because this is is update, and the messages are in fixedupate
    server.incoming_messages.clear();
    for (auto i = 0; i < server_messages.size(); i++)
      ImGui::Text("%s", server_messages[i].c_str());
  }

  if (client_entity != entt::null) {
    auto& client = r.get<SINGLETON_ClientComponent>(client_entity);
    ImGui::Text("You are a client connected to a server");
    SteamNetworkPingLocation_t ping;
    float result = SteamNetworkingUtils()->GetLocalPingLocation(ping);
    if (result == -1.0f)
      ImGui::Text("Your ping is: n/a");
    else
      ImGui::Text("Your ping is: %s", std::to_string(result).c_str());

    static char str0[128] = "Hello, world!";
    ImGui::InputText("input client", str0, IM_ARRAYSIZE(str0));
    if (ImGui::Button("Send to Server"))
      send_message_to_server(client, std::string(str0));

    //
    // View all incoming messages
    //
    ImGui::Separator();
    static std::vector<std::string> client_messages;
    client_messages.insert(client_messages.end(), client.messages.begin(), client.messages.end());
    // hack: because this is is update, and the messages are in fixedupate
    client.messages.clear();
    for (auto i = 0; i < client_messages.size(); i++)
      ImGui::Text("%s", client_messages[i].c_str());
  }

  ImGui::End();
}

} // namespace game2d