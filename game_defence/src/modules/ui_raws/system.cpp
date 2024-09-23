#include "system.hpp"

#include "actors/helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"

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

  static glm::vec2 spaceitem_pos{ 0, 0 };
  imgui_draw_vec2("item pos", spaceitem_pos);

  for (const auto& item : raws.items) {
    ImGui::Text("Item: %s", item.name.c_str());
    ImGui::SameLine();

    const std::string label = "world##" + item.name;
    if (ImGui::Button(label.c_str())) {
      auto e = spawn_item(r, item.name.c_str());
      set_position(r, e, spaceitem_pos);
    }

    ImGui::SameLine();
    const std::string label_player = "player##" + item.name;
    if (ImGui::Button(label_player.c_str())) {
      auto player_e = get_first<PlayerComponent>(r);

      // add item to the first player
      auto& inv_c = r.get<DefaultInventory>(player_e);
      for (size_t i = 0; i < inv_c.inv.size(); i++) {
        if (r.get<InventorySlotComponent>(inv_c.inv[i]).item_e == entt::null) {
          spawn_inv_item(r, inv_c.inv, i, item.name);
          break; // found a free slot
        }
      }
    }

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