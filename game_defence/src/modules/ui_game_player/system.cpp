#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/lifecycle/components.hpp"

// hmm, fix this include
#include "modules/ui_scene_main_menu/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_game_player_system(entt::registry& r){
  // ImGui::Begin("Player UI");
  // auto& ui = get_first_component<SINGLE_MainMenuUI>(r);

  // const auto& view = r.view<PlayerComponent, HealthComponent>(entt::exclude<WaitForInitComponent>);
  // for (int i = 0; const auto& [e, player, hp] : view.each()) {
  //   //
  //   ImGui::Text("Name: %s", ui.random_names[i].c_str());
  //   ImGui::Text("HP: %i/%i", hp.hp, hp.max_hp);
  //   ImGui::Text("Kills: %i", player.killed);

  //   i++;
  // }

  // ImGui::End();
};

} // namespace game2d