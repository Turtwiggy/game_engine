#include "system.hpp"

#include "modules/actor_spawner/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"

#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"
#include "resources/colours.hpp"

#include "imgui.h"
#include "magic_enum.hpp"

namespace game2d {

void
update_ui_spawner_system(entt::registry& r)
{
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  ImGui::Begin("Spawner Editor");

  const auto& view = r.view<SpawnerComponent, AttackCooldownComponent>();

  for (const auto& [entity, spawner, cooldown] : view.each()) {

    const EntityType& type = spawner.type_to_spawn;
    const auto type_name = std::string(magic_enum::enum_name(type));

    // std::vector<std::string> enemies{
    //   std::string(magic_enum::enum_name(EntityType::enemy_grunt)),
    //   std::string(magic_enum::enum_name(EntityType::enemy_shotgunner)),
    //   std::string(magic_enum::enum_name(EntityType::enemy_sniper)),
    // };
    // WomboComboIn in(enemies);
    // in.label = "select-mode";
    // in.current_index = mode_current_index;
    // const auto out = draw_wombo_combo(in);

    // grunt
    if (!spawner.class_set) {
      spawner.hp = 3;
      spawner.attack = 10;
      spawner.colour = colours.lin_yellow;
      spawner.sprite = "PERSON_25_1";
      spawner.class_set = true;
    }

    // update stats
    ImGui::InputFloat("Spawn Cooldown", &cooldown.time_between_attack, 0.1, 1.0);
    ImGui::InputInt("Unit HP", &spawner.hp, 1, 10);
    ImGui::InputInt("Unit Attack", &spawner.attack, 1, 10);

    // view stats
    ImGui::Text("¬¬");
    ImGui::Text("Currently spawning: %s", type_name.c_str());
    ImGui::Text("Cooldown: %f", cooldown.time_between_attack_left);
    ImGui::Text("HP: %i", spawner.hp);
    ImGui::Text("Attack: %i", spawner.attack);

    // grunt
    // hp 3
    // attack 10
    // colour = colours.lin_yellow;
    // sprite = "PERSON_25_1";

    // sniper
    // hp 3
    // attack 20
    // colour = colours.lin_blue;
    // sprite = "PERSON_25_6";

    // shotgunner
    // hp 4
    // attack 15
    // colour = colours.lin_orange;
    // sprite = "PERSON_28_1";
  }

  ImGui::End();
}

} // namespace game2d