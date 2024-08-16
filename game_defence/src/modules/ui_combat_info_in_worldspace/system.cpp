#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/system_fov/components.hpp"
#include "modules/ui_inventory/helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_combat_info_in_worldspace_system(entt::registry& r)
{
  // display all units hp/defence in worldspace
  // only display if the unit is hovered with a mouse
  // only display if the unit is visible in the fov system

  const auto& view = r.view<const HealthComponent, const DefenceComponent, const VisibleComponent>();
  for (const auto& [e, hp, defence, visible_c] : view.each()) {
    const auto size = get_size(r, e);

    auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
    // worldspace_ui.text = "HP:"s + std::to_string(hp.hp) + " DEF:" + std::to_string(defence.armour);
    worldspace_ui.offset.y = -1.25f * size.y; // place above

    // set imgui layout that gets positioned in worldspace

    worldspace_ui.requires_hovered = true;
    worldspace_ui.layout = [&r, &hp, &defence]() {
      const auto [heart_tl, heart_br] = convert_sprite_to_uv(r, "ICON_HEART_FULL");
      const auto [shield_tl, shield_br] = convert_sprite_to_uv(r, "shield_custom");
      const ImVec2 icon_size = { 16, 16 };
      const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
      const int custom_id = search_for_texture_id_by_texture_path(ri, "custom").value().id;
      const int kenny_id = search_for_texture_id_by_texture_path(ri, "monochrome").value().id;

      // icon + hp
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
      ImGui::Image((ImTextureID)kenny_id, icon_size, heart_tl, heart_br);
      ImGui::SameLine();
      ImGui::Text("%s", std::to_string(hp.hp).c_str());

      // uncomment StyleVar here if want space between icons
      ImGui::PopStyleVar();
      ImGui::SameLine();
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

      // icon + def
      ImGui::Image((ImTextureID)custom_id, icon_size, shield_tl, shield_br);
      ImGui::SameLine();
      ImGui::Text("%s", std::to_string(defence.armour).c_str());
      ImGui::PopStyleVar();
    };
  }
}

} // namespace game2d