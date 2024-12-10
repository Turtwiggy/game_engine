#include "system.hpp"

#include "modules/gameover/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_gameover_system(entt::registry& r)
{
  const auto& view = r.view<RequestGameOver>();
  bool new_game = view.size() > 0;
  r.destroy(view.begin(), view.end());

  // if (new_game)
  //   move_to_scene_start(r, Scene::menu);
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