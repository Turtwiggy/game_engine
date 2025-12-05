#include "pch.hpp"

#include "ui_debug_weapons_system.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_debug_weapons_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& weps_c = get_first_component<SINGLE_Weapons>(r);

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto state = gesert_menubar_state(menu_c, "Weapons");
  if (!state.enabled)
    return;

  ImGui::Begin("Weapons");

  ImGui::Text("Available Weapons: %i", (int)weps_c.weapons.size());

  for (const auto& wep : weps_c.weapons) {
    ImGui::Text("Weapon: %s", wep.name.c_str());

    ImGui::Text("Wep Audio");
    for (int i = 0; const auto& wep_audio : wep.audio) {
      // if (i > 0)
      // ImGui::SameLine();
      ImGui::Text("%s ", wep_audio.c_str());

      ImGui::SameLine();
      std::string button_tag = "Play##" + wep_audio;
      if (ImGui::Button(button_tag.c_str()))
        create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = wep_audio, .looping = false });

      i++;
    }
  }

  for (const auto& [e, weapon_b_c] : r.view<WeaponBehaviourComponent>().each()) {
    ImGui::Text("weapon.");
    for (const auto& wb : weapon_b_c.behaviours) {
      const auto enum_str = std::string(magic_enum::enum_name(wb));
      ImGui::Text("weapon behaviour: %s", enum_str.c_str());
    }
  }

  ImGui::NewLine();
  ImGui::SeparatorText("Weapon Upgrades");
  for (const auto& wep_upg : weps_c.weapon_upgrades) {
    ImGui::Text("%s %s", wep_upg.wb_key.c_str(), wep_upg.desc.c_str());
    for (const auto& stat : wep_upg.stats)
      ImGui::Text("%s", stat.stat.c_str());
  }

  ImGui::End();
}

} // namespace game2d