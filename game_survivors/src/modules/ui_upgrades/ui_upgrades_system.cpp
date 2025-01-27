#include "ui_upgrades_system.hpp"

#include "engine/enum/enum_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_upgrades_system(entt::registry& r)
{
  ImGui::Begin("Upgrades");

  static auto mode = UpgradeableStat::BULLET_DAMAGE;
  static auto modes = engine::enum_class_to_vec_str<UpgradeableStat>();
  WomboComboIn combo_in(modes);
  combo_in.label = "mod";
  combo_in.current_index = static_cast<int>(mode);
  WomboComboOut combo_out = draw_wombo_combo(combo_in);
  if (combo_in.current_index != combo_out.selected)
    mode = static_cast<UpgradeableStat>(combo_out.selected);

  auto modifier = std::string(magic_enum::enum_name(mode));

  static float mod_val = 10;
  imgui_draw_float("mod_val", mod_val);

  static std::string trait = "assassin";
  imgui_draw_string("trait", trait);

  const bool add_flat = ImGui::Button("Add Flat");
  const bool add_percent = ImGui::Button("Add Percentage");
  const bool add_trait = ImGui::Button("Add Trait");

  const auto& view = r.view<TagComponent, StatModifierComponent, TraitComponent>();
  for (const auto [e, tag_c, stat_c, traits_c] : view.each()) {
    ImGui::Text("Tag: %s", tag_c.tag.c_str());

    ImGui::Text("Modifiers...");
    for (const auto& mod : stat_c.modifiers)
      ImGui::Text("%s, %s", mod->stat.c_str(), mod->modifier.c_str());

    ImGui::Text("Traits...");
    for (const auto& trait : traits_c.traits)
      ImGui::Text("%s", trait.key.c_str());

    if (add_flat)
      stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, modifier));

    if (add_percent)
      stat_c.add(std::make_shared<StatPercentIncrease>(mod_val, modifier));

    if (add_trait)
      traits_c.traits.push_back(Trait{ trait });
  }

  ImGui::End();
}

} // namespace game2d