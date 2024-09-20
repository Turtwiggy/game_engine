#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actor_cargo/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_gameover/components.hpp"

#include <algorithm>

namespace game2d {
using namespace std::literals;

void
update_gameover_system(entt::registry& r)
{
  //
  // gameover conditions
  //
  {
    auto& gameover = get_first_component<SINGLE_GameOver>(r);
    const auto& scene = get_first_component<SINGLE_CurrentScene>(r);

    // Scenes to add gameover condition to
    const auto s = std::vector<Scene>{ Scene::dungeon_designer };
    const auto it = std::find_if(s.begin(), s.end(), [&scene](const Scene& sc) { return sc == scene.s; });
    if (it != s.end()) {
      // Make sure we're not generating a dungeon...
      // Where players are temporarily 0...
      const bool generated_dungeon = get_first<DungeonGenerationResults>(r) != entt::null;
      if (generated_dungeon) {

        // condition: All of one team is daed
        std::map<AvailableTeams, int> team_count;
        team_count[AvailableTeams::player] = 0; // force team to exist

        // Count up all teams
        for (const auto& [e, team_c] : r.view<TeamComponent>().each())
          team_count[team_c.team] += 1;
        if (team_count[AvailableTeams::player] == 0) {
          gameover.game_is_over = true;
          gameover.win_condition = false;
          gameover.reason = "Your team wiped out!";

          // punish the player! make them lose a cargo!
          decrement_cargo(r);
        }
      }
    }

    if (gameover.game_is_over && !gameover.activated_gameover) {
      gameover.activated_gameover = true;

      if (gameover.win_condition) {
        // WHOOOOOOOOOOOO!
        // create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "WIN_01" });
      } else {
        // WAHHHHHHHHHHHHH.
        // create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "LOSS_01" });
      }
    }
  }

  // gameover requests
  //
  bool new_game = false;
  {
    const auto& view = r.view<NewGameRequest>(entt::exclude<WaitForInitComponent>);
    for (const auto& [entity, request] : view.each())
      new_game = true;
    r.destroy(view.begin(), view.end()); // requests are processed
  }

  // do the restart
  // bug: should not be scene::
  if (new_game)
    move_to_scene_start(r, Scene::menu, false);
}

} // namespace game2d

// HACK: Check if the game is over (base explodes)
//     const auto& first_hearth = get_first<HearthComponent>(r);
//     if (first_hearth != entt::null) {
//       const auto& first_hearth_hp = r.get<HealthComponent>(first_hearth);
//       if (first_hearth_hp.hp <= 0) {
//         gameover.game_is_over = true;
//         gameover.win_condition = false;
//         gameover.reason = "Your hearth exploded!";
//       }
//     }

// HACK: Check if the game is over (all players are ded)
//     const auto& players_view = r.view<PlayerComponent>();
//     const bool all_players_ded = players_view.size() == 0;
//     if (all_players_ded) {
//       gameover.game_is_over = true;
//       gameover.win_condition = false;
//       gameover.reason = "All Players ded!";
//     }

// HACK: Check if the game is over (you beat wave 10)
// const auto& wave = get_first_component<SINGLE_Wave>(r);
// if (wave.wave == 10) {
//   gameover.game_is_over = true;
//   gameover.reason = "You made it to wave 10!";
// }

// HACK: gameover condition: All enemies are dead and no spawners
// const bool no_enemies = r.view<EnemyComponent>().size() == 0;
// bool no_enemy_spawners = true;
// {
//   for (const auto& [e, spawner] : r.view<SpawnerComponent>().each()) {
//     // if (spawner.type_to_spawn == EntityType::enemy_grunt)
//     no_enemy_spawners = false;
//   }
// }
// if (no_enemies && no_enemy_spawners) {
//   gameover.game_is_over = true;
//   gameover.win_condition = true;
//   gameover.reason = "Level Complete! You survived with "s + std::to_string(players_view.size()) + " players"s;
// }

// HACK: gameover condition: X minutes survived
// {
//   static int minutes = 1;
//   static float gameover_cooldown = minutes * 60;
//   static float gameover_cooldown_left = gameover_cooldown;
//   gameover_cooldown_left -= dt;
//   ImGui::Begin("Gameover");
//   ImGui::Text("Survive: %f", gameover_cooldown_left);
//   ImGui::End();
//   if (gameover_cooldown_left <= 0.0f) {
//     gameover.game_is_over = true;
//     gameover.win_condition = true;
//     gameover.reason = std::string("You escaped! Survived for ") + std::to_string(minutes) + std::string(" minutes");
//   }
//   if (gameover.game_is_over) {
//     ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking;
//     flags |= ImGuiWindowFlags_NoFocusOnAppearing;
//     ImGui::Begin("Escape!", NULL, flags);
//     ImGui::Text("You escaped!");
//     ImGui::End();
//   }
// }