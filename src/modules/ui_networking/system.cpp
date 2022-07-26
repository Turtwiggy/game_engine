#include "system.hpp"

#include "modules/ui_networking/components.hpp"

#include <imgui.h>

namespace game2d {

void
init_ui_networking_system(entt::registry& r)
{
  r.ctx().emplace<SINGLETON_NetworkingUIComponent>();
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

  ImGui::End();
}

} // namespace game2d