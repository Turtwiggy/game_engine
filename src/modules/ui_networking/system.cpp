#include "system.hpp"

#include "modules/networking/components.hpp"
#include "modules/ui_networking/components.hpp"

#include <imgui.h>
#include <steam/isteamnetworkingutils.h>

namespace game2d {

void
init_ui_networking_system(entt::registry& r)
{
  r.ctx().emplace<SINGLETON_NetworkingUIComponent>();

  auto& ui = r.ctx().at<SINGLETON_NetworkingUIComponent>();
  ui.utils = SteamNetworkingUtils();
}

void
update_ui_networking_system(entt::registry& r)
{
  auto& ui = r.ctx().at<SINGLETON_NetworkingUIComponent>();

  ImGui::Begin("Networking");

  if (ImGui::Button("Start Server"))
    ui.start_server = true;

  if (ImGui::Button("Start Client"))
    ui.start_client = true;

  if (ImGui::Button("Stop Networking"))
    ui.close_networking = true;

  if (r.ctx().contains<SINGLETON_ServerComponent>()) {
    auto& server = r.ctx().at<SINGLETON_ServerComponent>();
    ImGui::Text("You are a server");
    ImGui::Text("You have %i clients", server.clients.size());
  }

  if (r.ctx().contains<SINGLETON_ClientComponent>()) {
    auto& client = r.ctx().at<SINGLETON_ClientComponent>();
    ImGui::Text("You are a client connected to a server");

    SteamNetworkPingLocation_t ping;
    float result = ui.utils->GetLocalPingLocation(ping);
    if (result == -1.0f)
      ImGui::Text("Your ping is: n/a");
    else
      ImGui::Text("Your ping is: %s", std::to_string(result).c_str());
  }

  ImGui::End();
}

} // namespace game2d