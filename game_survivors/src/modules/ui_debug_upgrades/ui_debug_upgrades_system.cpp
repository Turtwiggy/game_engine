#include "ui_debug_upgrades_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/enum/enum_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade/upgrade_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"


#include <imgui.h>

#include <format>

namespace game2d {

void
update_ui_debug_upgrades_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, up_e, up_c);
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto state = gesert_menubar_state(menu_c, "Upgrades");
  if (!state.enabled)
    return;
  ImGui::Begin(state.name.c_str());

  ImGui::SeparatorText("Upgrades");

  //
  // Convert upgrades to vec<std::string> of their keys, and display them in wombocombo
  //
  static int index = 0;
  auto keys = available_upgrade_names(r);
  {
    WomboComboIn combo_in(keys);
    combo_in.label = "upgrades";
    combo_in.current_index = static_cast<int>(index);
    WomboComboOut combo_out = draw_wombo_combo(combo_in);
    if (combo_in.current_index != combo_out.selected)
      index = combo_out.selected;
  }

  //
  // display the selected upgrade info
  //
  if (up_c.upgrades.size() > 0) {
    Upgrade& u = up_c.upgrades[index];

    std::string desc = generate_description(u);
    ImGui::Text("%s", desc.c_str());

    std::string label = "Aquire##" + u.name;
    if (ImGui::Button(label.c_str())) {
      for (const auto& [e, player_c] : r.view<const PlayerComponent>().each()) {
        // send event, me thinks
        UpgradeEvent evt;
        evt.e = e;
        evt.upgrade = u;
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();
      }
    }
  }

  if (ImGui::Button("AquireAll")) {
    for (const auto& [e, player_c] : r.view<const PlayerComponent>().each()) {
      for (const auto& u : up_c.upgrades) {
        UpgradeEvent evt;
        evt.e = e;
        evt.upgrade = u;
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();
      }
    }
  }

  ImGui::SeparatorText("Modifier Config");
  static auto stat_mode = UpgradeableStat::BULLET_DAMAGE;
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

  ImGui::SeparatorText("Trait Config");
  static auto trait_mode = AquirableTrait::ASSASSIN;
  static auto trait_modes = engine::enum_class_to_vec_str<AquirableTrait>();
  {
    WomboComboIn combo_in(trait_modes);
    combo_in.label = "trait";
    combo_in.current_index = static_cast<int>(trait_mode);
    WomboComboOut combo_out = draw_wombo_combo(combo_in);
    if (combo_in.current_index != combo_out.selected)
      trait_mode = static_cast<AquirableTrait>(combo_out.selected);
  }
  const auto trait = std::string(magic_enum::enum_name(trait_mode));
  const bool add_trait = ImGui::Button("Add Trait");

  const auto& view = r.view<TagComponent, StatModifierComponent, TraitComponent>();
  for (const auto [e, tag_c, stat_c, traits_c] : view.each()) {
    ImGui::SeparatorText(std::format("{}", tag_c.tag).c_str());

    for (const auto& mod : stat_c.modifiers)
      ImGui::Text("Mod: %s, %s", mod->stat.c_str(), mod->modifier.c_str());

    for (const auto& trait : traits_c.traits)
      ImGui::Text("Trait: %s", std::string(magic_enum::enum_name(trait)).c_str());

    if (add_flat)
      stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, modifier));

    if (add_percent)
      stat_c.add(std::make_shared<StatPercentIncrease>(mod_val, modifier));

    if (add_trait)
      traits_c.traits.emplace(trait_mode);
  }

  ImGui::End();
}

} // namespace game2d