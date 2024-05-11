#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"

#include "entt/helpers.hpp"
#include "imgui.h"

namespace game2d {

void
update_ui_dungeon_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& data_e = get_first<SINGLE_DuckgameToDungeon>(r);

  ImGui::Begin("UIDungeonScene");

  if (data_e != entt::null) {
    // you arrived from a scene
    const auto& data_c = r.get<SINGLE_DuckgameToDungeon>(data_e);
    ImGui::Text("Somethingwasstabbedtogethere: %i", data_c.backstabbed);

    if (data_c.backstabbed) {
      ImGui::Text("You've ambushed something! You win!");
      if (ImGui::Button("Back to duckscene")) {

        // TODO: need to reload duckscene state...

        // going to "duckscene" scene
        move_to_scene_start(r, Scene::duckgame_overworld);
      }
    }
  } else {
    // you clicked this scene as a standalone scene!

    ImGui::Text("standalone scene..!");
    if (ImGui::Button("Generate Dungeon")) {
      generate_dungeon(r);

      // HACK: spawn a patrol to test pathfinding
      const auto enemy = create_gameplay(r, EntityType::actor_enemy_patrol);
      // random position, dont spawn at 0, 0
      const int rnd_x = int(5);
      const int rnd_y = int(5);
      set_position(r, enemy, { rnd_x * 50, rnd_y * 50 });
    }

    if (ImGui::Button("Back to duckscene")) {
      // TODO: need to reload duckscene state...
    }

    // HACK: if you clicked rmb, move the player there
    if (get_mouse_rmb_press()) {
      const auto player_e = get_first<PlayerComponent>(r);
      set_position(r, player_e, mouse_pos);
    }
  }

  ImGui::End();
};

} // namespace game2d