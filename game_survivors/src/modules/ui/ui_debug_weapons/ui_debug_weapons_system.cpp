#include "pch.hpp"

#include "ui_debug_weapons_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/combat/combat_core/combat_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_debug_weapons_system(entt::registry& r)
{
  const auto& weps_c = get_first_component<SINGLE_Weapons>(r);

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto state = gesert_menubar_state(menu_c, "DebugWeapons");
  if (!state.enabled)
    return;

  ImGui::Begin("DebugWeapons");

  ImGui::Text("Available Weapons: %i", (int)weps_c.weapons.size());

  for (const auto& wep : weps_c.weapons) {
    ImGui::Text("Weapon: %s", wep.name.c_str());

    // for (const auto& [key, val] : wep.data)
    //   ImGui::Text("%s,%f", key.c_str(), val);

    const std::string equip_label = "Equip##" + wep.key;
    if (ImGui::Button(equip_label.c_str())) {
      // update your weapon stats...
      SDL_Log("Equipping... %s", wep.key.c_str());
    }
  }

  ImGui::End();
}

} // namespace game2d