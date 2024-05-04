#include "system.hpp"

#include "components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"

#include "entt/helpers.hpp"
#include "imgui.h"

namespace game2d {

void
update_ui_dungeon_system(entt::registry& r)
{
  const auto& previous_scene_data = get_first_component<SINGLE_DuckgameToDungeon>(r);

  ImGui::Begin("UIDungeonScene");

  ImGui::Text("Somethingwasstabbedtogethere: %i", previous_scene_data.backstabbed);
  if (previous_scene_data.backstabbed) {
    ImGui::Text("You've ambushed something! You win!");
    if (ImGui::Button("Back to duckscene")) {

      // TODO: need to reload duckstene state...

      // going to "duckscene" scene
      move_to_scene_start(r, Scene::duckgame);
    }
  }

  ImGui::End();
};

} // namespace game2d