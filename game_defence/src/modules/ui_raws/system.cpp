#include "system.hpp"

#include "modules/raws_components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_raws_system(entt::registry& r)
{
  const auto& raws = get_first_component<Raws>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDocking;

  ImGui::Begin("raws", NULL, flags);

  std::string items_header = fmt::format("items: {}", raws.items.size());
  ImGui::SeparatorText(items_header.c_str());

  for (const auto& item : raws.items) {
    ImGui::Text("Item: %s", item.name.c_str());
    ImGui::SameLine();

    const std::string label = "spawn##" + item.name;
    if (ImGui::Button(label.c_str()))
      auto e = spawn_item(r, item.name.c_str());
    if (item.use.has_value()) {
      ImGui::SameLine();
      ImGui::Text("(has use...)");
    }
  }
  ImGui::Text("environment: %zu", raws.environment.size());
  ImGui::Text("mobs: %zu", raws.mobs.size());

  ImGui::End();
};

} // namespace game2d