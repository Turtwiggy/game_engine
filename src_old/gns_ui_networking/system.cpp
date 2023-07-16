#include "system.hpp"

#include "networking/components.hpp"
#include "ui_networking/components.hpp"

#include <imgui.h>
#include <steam/isteamnetworkingutils.h>

namespace game2d {

void
update_ui_networking_system(GameEditor& editor, Game& game)
{
  auto& ui = editor.networking_ui;

  if (ui.start_offline)
    return; // dont show networking panel if playing offline

  ImGui::Begin("Networking");

  if (ImGui::Button("Start Server"))
    ui.start_server = true;

  if (ImGui::Button("Start Client"))
    ui.start_client = true;

  if (ImGui::Button("Stop Networking"))
    ui.close_networking = true;

  if (ImGui::Button("Start Offline"))
    ui.start_offline = true;

  // if (r.ctx().contains<SINGLETON_ServerComponent>()) {
  //   auto& server = r.ctx().at<SINGLETON_ServerComponent>();
  //   ImGui::Text("You are a server");
  //   ImGui::Text("You have %i clients", server.clients.size());
  // }

  // if (r.ctx().contains<SINGLETON_ClientComponent>()) {
  //   auto& client = r.ctx().at<SINGLETON_ClientComponent>();
  //   ImGui::Text("You are a client connected to a server");

  //   SteamNetworkPingLocation_t ping;
  //   float result = SteamNetworkingUtils()->GetLocalPingLocation(ping);
  //   if (result == -1.0f)
  //     ImGui::Text("Your ping is: n/a");
  //   else
  //     ImGui::Text("Your ping is: %s", std::to_string(result).c_str());
  // }

  ImGui::End();
}

} // namespace game2d