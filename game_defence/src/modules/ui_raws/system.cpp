#include "system.hpp"

#include "modules/raws/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_raws_system(entt::registry& r)
{
  const auto& raws = get_first_component<Raws>(r);

  ImGui::Begin("raws");

  std::string items_header = fmt::format("items: {}", raws.items.size());
  ImGui::SeparatorText(items_header.c_str());

  for (const auto& item : raws.items) {
    ImGui::Text("Item: %s", item.name.c_str());
    ImGui::SameLine();

    const std::string label = "spawn##" + item.name;
    if (ImGui::Button(label.c_str()))
      auto e = spawn_item(r, item.name.c_str(), { 50, 50 });
    if (item.use.has_value()) {
      ImGui::SameLine();
      ImGui::Text("(has use...)");
    }
  }

  ImGui::Text("weapons: %zu", raws.weapons.size());
  ImGui::Text("armour: %zu", raws.armour.size());
  ImGui::Text("environment: %zu", raws.environment.size());
  ImGui::Text("mobs: %zu", raws.mobs.size());

  ImGui::End();
};

} // namespace game2d