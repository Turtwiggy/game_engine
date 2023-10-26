#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/items/helpers.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

#include "entt/entt.hpp"
#include "glm/glm.hpp"
#include <imgui.h>

#include <string>

namespace game2d {
using namespace std::literals;

void
update_ui_dropoff_zone_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  const auto& view = r.view<DropoffZoneComponent, AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, zone, aabb] : view.each()) {

    const auto eid = std::to_string(static_cast<uint32_t>(entity));
    std::string beginlabel = "DropoffZone##"s + eid;

    const auto worldspace = position_in_worldspace(r, aabb.center);
    ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };

    // offset
    // pos.y -= aabb.size.y;
    pos.y -= 100.0f;

    pos.x = glm::clamp(static_cast<int>(pos.x), 0, ri.viewport_size_render_at.x);
    pos.y = glm::clamp(static_cast<int>(pos.y), 0, ri.viewport_size_render_at.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // ImGui::PushStyleColor(ImGuiCol_, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiDockNodeFlags_AutoHideTabBar;
    flags |= ImGuiDockNodeFlags_NoResize;

    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    // ImGui::PopStyleColor(1);

    // ImGui::Text("Pos %f %f", pos.x, pos.y);

    for (int i = 0; i < zone.requested_items.size(); i++) {
      if (i > 0)
        ImGui::SameLine();
      const auto info = item_id_to_sprite(r, zone.requested_items[i]);
      ImGui::Text("%s", info.display.c_str());
    }

    ImGui::NewLine();
    std::string add = "+##"s + eid;
    if (ImGui::Button(add.c_str())) {
      zone.cur_customers++;
      zone.cur_customers = glm::clamp(zone.cur_customers, 0, zone.max_customers);
      std::cout << "newcustomers: " << zone.cur_customers << std::endl;
    }

    ImGui::SameLine();
    std::string del = "-##"s + eid;
    if (ImGui::Button(del.c_str())) {
      zone.cur_customers--;
      zone.cur_customers = glm::clamp(zone.cur_customers, 0, zone.max_customers);
      std::cout << "newcustomers: " << zone.cur_customers << std::endl;
    }

    ImGui::End();
  }
}

} // namespace game2d