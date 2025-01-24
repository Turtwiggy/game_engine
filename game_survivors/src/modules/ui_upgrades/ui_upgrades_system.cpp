#include "ui_upgrades_system.hpp"

#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_upgrades_system(entt::registry& r)
{
  const auto& view = r.view<TagComponent, StatModifierComponent>();

  ImGui::Begin("Upgrades");

  static std::string modifier = "bullet_speed";
  imgui_draw_string("modifier", modifier);

  static float mod_val = 10;
  imgui_draw_float("mod_val", mod_val);

  static std::string trait = "assassin";
  imgui_draw_string("trait", trait);

  for (const auto [e, tag_c, stat_c] : view.each()) {
    ImGui::Text("Tag: %s", tag_c.tag.c_str());

    if (ImGui::Button("Add Flat"))
      stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, modifier));

    if (ImGui::Button("Add Percentage"))
      stat_c.add(std::make_shared<StatPercentIncrease>(mod_val, modifier));

    ImGui::Text("Modifiers: ");
    for (const auto& mod : stat_c.modifiers)
      ImGui::Text("%s, %s", mod->stat.c_str(), mod->modifier.c_str());

    // get base bullet_speed
    float example_base_bullet_speed = 250;
    ImGui::Text("Base Bullet Speed: %f", example_base_bullet_speed);
    ImGui::Text("Modified Bullet Speed: %f", stat_c.apply_modifiers(example_base_bullet_speed, "bullet_speed"));

    ImGui::Text("Traits...");
    if (auto* traits_c = r.try_get<TraitComponent>(e)) {
      for (const auto& trait : traits_c->traits)
        ImGui::Text("%s", trait.key.c_str());

      if (ImGui::Button("Add Trait"))
        traits_c->traits.push_back(Trait{ trait });
    }
  }

  ImGui::End();
}

} // namespace game2d