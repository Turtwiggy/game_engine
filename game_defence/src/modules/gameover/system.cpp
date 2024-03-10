#include "system.hpp"

#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_gameover/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_gameover_system(entt::registry& r)
{
  //
  // gameover conditions
  //
  {
    auto& gameover = get_first_component<SINGLETON_GameOver>(r);
    const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);

    // this if statement seems like its in the wrong place
    if (scene.s == Scene::game) {

      // Check if the game is over (base explodes)
      const auto& first_hearth = get_first<HearthComponent>(r);
      if (first_hearth != entt::null) {
        const auto& first_hearth_hp = r.get<HealthComponent>(first_hearth);
        if (first_hearth_hp.hp <= 0) {
          gameover.game_is_over = true;
          gameover.win_condition = false;
          gameover.reason = "Your hearth exploded!";
        }
      }

      // Check if the game is over (all players are ded)
      const auto& players_view = r.view<PlayerComponent>();
      const bool all_players_ded = players_view.size() == 0;
      if (all_players_ded) {
        gameover.game_is_over = true;
        gameover.win_condition = false;
        gameover.reason = "All Players ded!";
      }

      // Check if the game is over (you beat wave 10)
      // const auto& wave = get_first_component<SINGLETON_Wave>(r);
      // if (wave.wave == 10) {
      //   gameover.game_is_over = true;
      //   gameover.reason = "You made it to wave 10!";
      // }

      // Check if all enemies are dead, and there are no spawners
      const bool no_enemies = r.view<EnemyComponent>().size() == 0;
      bool no_enemy_spawners = true;
      {
        for (const auto& [e, spawner] : r.view<SpawnerComponent>().each()) {
          // if (spawner.type_to_spawn == EntityType::enemy_grunt)
          no_enemy_spawners = false;
        }
      }

      if (no_enemies && no_enemy_spawners) {
        gameover.game_is_over = true;
        gameover.win_condition = true;
        gameover.reason = "Level Complete! You survived with "s + std::to_string(players_view.size()) + " players"s;
      }
    }

    if (gameover.game_is_over && !gameover.activated_gameover) {
      gameover.activated_gameover = true;

      if (gameover.win_condition) {
        stop_all_audio(r);
        // WHOOOOOOOOOOOO!
        r.emplace<AudioRequestPlayEvent>(r.create(), "WIN_01");
      } else {
        stop_all_audio(r);
        // WAHHHHHHHHHHHHH.
        r.emplace<AudioRequestPlayEvent>(r.create(), "LOSS_01");
      }
    }
  }

  //
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
  if (new_game)
    move_to_scene_start(r, Scene::game);
}

} // namespace game2d