#include "ui_upgrades_system.hpp"

#include "engine/enum/enum_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <imgui.h>

#include <format>

namespace game2d {

void
update_ui_upgrades_system(entt::registry& r)
{
  ImGui::Begin("Upgrades");

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
      traits_c.traits.push_back(trait_mode);
  }

  ImGui::End();
}

} // namespace game2d