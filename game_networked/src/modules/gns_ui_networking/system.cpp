#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/gns_networking_client/components.hpp"
#include "modules/gns_networking_server/components.hpp"
#include "modules/gns_ui_networking/components.hpp"

#include <imgui.h>
#include <steam/isteamnetworkingutils.h>

namespace game2d {

void
update_ui_networking_system(entt::registry& r)
{
  auto& ui = get_first_component<SINGLETON_NetworkingUIComponent>(r);

  ImGui::Begin("Networking");

  if (ImGui::Button("Start Server"))
    ui.start_server = true;

  if (ImGui::Button("Start Client"))
    ui.start_client = true;

  if (ImGui::Button("Stop Networking"))
    ui.close_networking = true;

  if (ImGui::Button("Start Offline"))
    ui.start_offline = true;

  const auto server_entity = get_first<SINGLETON_ServerComponent>(r);
  if (server_entity != entt::null) {
    const auto server = r.get<SINGLETON_ServerComponent>(server_entity);
    ImGui::Text("You are a server");
    ImGui::Text("You have %i clients", server.clients.size());
  }

  const auto client_entity = get_first<SINGLETON_ServerComponent>(r);
  if (client_entity != entt::null) {
    const auto client = r.get<SINGLETON_ClientComponent>(client_entity);
    ImGui::Text("You are a client connected to a server");
    SteamNetworkPingLocation_t ping;
    float result = SteamNetworkingUtils()->GetLocalPingLocation(ping);
    if (result == -1.0f)
      ImGui::Text("Your ping is: n/a");
    else
      ImGui::Text("Your ping is: %s", std::to_string(result).c_str());
  }

  ImGui::End();
}

} // namespace game2d