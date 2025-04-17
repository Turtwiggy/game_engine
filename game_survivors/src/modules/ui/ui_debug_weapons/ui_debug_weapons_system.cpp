#include "pch.hpp"

#include "ui_debug_weapons_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_debug_weapons_system(entt::registry& r)
{
  const auto& weps_c = get_first_component<SINGLE_Weapons>(r);

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto state = gesert_menubar_state(menu_c, "Weapons");
  if (!state.enabled)
    return;

  ImGui::Begin("Weapons");

  ImGui::Text("Available Weapons: %i", (int)weps_c.weapons.size());

  for (const auto& wep : weps_c.weapons) {
    ImGui::Text("Weapon: %s", wep.name.c_str());

    for (const std::string& wep_upg : wep.upgrades)
      ImGui::Text("Wep Upgrade Key: %s", wep_upg.c_str());
  }

  ImGui::SeparatorText("Weapon Upgrades");
  for (const auto& wep_upg : weps_c.weapon_upgrades) {
    ImGui::Text("%s %s %s", wep_upg.key.c_str(), wep_upg.game_key.c_str(), wep_upg.desc.c_str());
    for (const auto& stat : wep_upg.stats)
      ImGui::Text("%s", stat.stat.c_str());
  }

  ImGui::End();
}

} // namespace game2d