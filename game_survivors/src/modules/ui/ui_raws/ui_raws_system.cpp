#include "modules/ui/ui_raws/ui_raws_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

#include <imgui.h>

#include <format>

namespace game2d {

void
update_ui_raws_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& raws = get_first_component<Raws>(r);

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto ui_state = gesert_menubar_state(menu_c, "Raws");
  if (!ui_state.enabled)
    return;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  ImGui::Begin("raws", NULL, flags);

  std::string items_header = std::format("items: {}", raws.items.size());
  ImGui::SeparatorText(items_header.c_str());

  static glm::vec2 spaceitem_pos{ 0, 0 };
  imgui_draw_vec2("item pos", spaceitem_pos);

  for (const auto& item : raws.items) {
    ImGui::Text("Item: %s", item.name.c_str());
    ImGui::SameLine();

    const std::string label = "world##" + item.name;
    if (ImGui::Button(label.c_str())) {
      auto e = spawn(r, item.name.c_str());
      give_life(r, e, spaceitem_pos);

      // hack: add a piece of scrap to a lootbag
      // if (item.name.find("lootbag") != std::string::npos) {
      //   auto& inv_c = r.get<DefaultInventory>(e);
      //   spawn_inv_item(r, inv_c.inv, 0, "scrap");
      // }
    }

    ImGui::SameLine();
    const std::string label_player = "player##" + item.name;
    if (ImGui::Button(label_player.c_str())) {
      auto player_e = get_first<PlayerComponent>(r);
    }

    if (item.use.has_value()) {
      ImGui::SameLine();
      ImGui::Text("(has use...)");
    }
  }

  ImGui::End();
};

} // namespace game2d