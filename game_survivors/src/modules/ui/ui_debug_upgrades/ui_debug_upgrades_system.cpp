#include "pch.hpp"

#include "ui_debug_upgrades_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/event_weapon_level_reached/event_weapon_level_reached_helpers.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_helpers.hpp"
#include "modules/systems/system_traits/trait_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

namespace game2d {

bool
draw_debug_ai_behaviours_ui(entt::registry& r)
{
  ImGui::SeparatorText("Trait Config");
  static auto trait_mode = magic_enum::enum_value<AiBehaviour>(0);
  static auto trait_modes = engine::enum_class_to_vec_str<AiBehaviour>();
  {
    WomboComboIn combo_in(trait_modes);
    combo_in.label = "trait";
    combo_in.current_index = static_cast<int>(trait_mode);
    WomboComboOut combo_out = draw_wombo_combo(combo_in);
    if (combo_in.current_index != combo_out.selected)
      trait_mode = static_cast<AiBehaviour>(combo_out.selected);
  }
  const auto trait = std::string(magic_enum::enum_name(trait_mode));
  const bool add_trait = ImGui::Button("Add Trait");
  return add_trait;
};

std::pair<bool, bool>
draw_debug_modifier_ui(entt::registry& r)
{
  ImGui::SeparatorText("Modifier Config");
  static auto stat_mode = magic_enum::enum_value<UpgradeableStat>(0);
  static auto stat_modes = engine::enum_class_to_vec_str<UpgradeableStat>();
  {
    WomboComboIn combo_in(stat_modes);
    combo_in.label = "mod";
    combo_in.current_index = static_cast<int>(stat_mode);
    WomboComboOut combo_out = draw_wombo_combo(combo_in);
    if (combo_in.current_index != combo_out.selected)
      stat_mode = static_cast<UpgradeableStat>(combo_out.selected);
  }
  const auto modifier = std::string(magic_enum::enum_name(stat_mode));
  static float mod_val = 1.0f;
  imgui_draw_float("mod_val", mod_val);

  const bool add_flat = ImGui::Button("Add Flat");
  ImGui::SameLine();
  const bool add_percent = ImGui::Button("Add Percentage");

  const auto& view = r.view<TagComponent, StatModifierComponent, WeaponBehaviourComponent>();
  for (const auto [e, tag_c, stat_c, wb_c] : view.each()) {
    ImGui::SeparatorText(std::format("{}", tag_c.tag).c_str());

    for (const auto& mod : stat_c.modifiers) {
      auto type = mod->modifier; // flat or percent
      if (type == "stat_percent_increase") {
        const auto* ptr = dynamic_cast<StatPercentIncrease*>(mod.get());
        ImGui::Text("%s, %0.2f", mod->stat.c_str(), ptr->percent);
      } else if (type == "stat_flat_increase") {
        const auto* ptr = dynamic_cast<StatFlatIncrease*>(mod.get());
        ImGui::Text("%s, %0.2f", mod->stat.c_str(), ptr->increase);
      }
    }

    for (const auto& wb : wb_c.behaviours)
      ImGui::Text("WepMod: %s", std::string(magic_enum::enum_name(wb)).c_str());

    if (add_flat)
      stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, modifier));

    if (add_percent)
      stat_c.add(std::make_shared<StatPercentIncrease>(mod_val, modifier));
  }

  return { add_flat, add_percent };
};

void
update_ui_debug_upgrades_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, up_e, up_c);
  auto& evts_c = SINGLE_Events::instance;

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto state = gesert_menubar_state(menu_c, "Upgrades");
  if (!state.enabled)
    return;
  ImGui::Begin(state.name.c_str());

  ImGui::SeparatorText("WeaponBehaviours");

  static WeaponBehaviour wep_behaviour = magic_enum::enum_value<WeaponBehaviour>(0);
  static auto wep_behaviours = engine::enum_class_to_vec_str<WeaponBehaviour>();
  {
    WomboComboIn combo_in(wep_behaviours);
    combo_in.label = "weapon behaviours";
    combo_in.current_index = static_cast<int>(wep_behaviour);
    WomboComboOut combo_out = draw_wombo_combo(combo_in);
    if (combo_in.current_index != combo_out.selected)
      wep_behaviour = static_cast<WeaponBehaviour>(combo_out.selected);
  }

  const bool add_wep_behaviour = ImGui::Button("Add Weapon Behaviour");
  if (add_wep_behaviour) {
    for (const auto& [e, player_c] : r.view<const PlayerComponent>().each()) {

      UpgradeRollResult roll;
      roll.rarity = Rarity::COMMON;
      roll.stats = get_stats_from_weapon_behaviour(r, wep_behaviour);
      roll.traits = { wep_behaviour };
      roll.weapons = get_weapons(r, e);
      roll.level_weapons = true;

      UpgradeEvent evt;
      evt.par_e = e;
      evt.upg_e = e; // add to player
      evt.roll_result = roll;
      evts_c.dispatcher->trigger(evt);
    }

    evts_c.dispatcher->update();
  }

  const auto [add_flat, add_percent] = draw_debug_modifier_ui(r);
  // const auto add_trait = draw_debug_trait_ui(r);

  ImGui::End();
}

} // namespace game2d