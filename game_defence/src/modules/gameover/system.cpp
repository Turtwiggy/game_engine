#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/combat/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/hearth/components.hpp"
#include "modules/player/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/spawner/components.hpp"
#include "modules/ui_gameover/components.hpp"

namespace game2d {

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
          gameover.reason = "Your hearth exploded!";
        }
      }

      // Check if the game is over (all players are ded)
      const auto& first_player = get_first<PlayerComponent>(r);
      if (first_player != entt::null) {
        const auto& health = r.try_get<HealthComponent>(first_player);
        if (health && health->hp <= 0) {
          gameover.game_is_over = true;
          gameover.reason = "Player ded!";
        }
      }

      // Check if the game is over (all spanwers are ded)

      bool enemy_spawner_exists = false;
      const auto& spawners = r.view<SpawnerComponent>();
      for (const auto [entity, spawner] : spawners.each()) {
        enemy_spawner_exists |= spawner.type_to_spawn == EntityType::actor_enemy;
      }

      if (!enemy_spawner_exists) {
        gameover.game_is_over = true;
        gameover.reason = "Defeated all spawners!";
      }
    }
  }

  //
  // gameover requests
  //
  bool new_game = false;
  {
    const auto& view = r.view<NewGameRequest>();
    new_game = view.size() > 0;
    for (const auto& [entity, req] : view.each())
      r.destroy(entity);
  }
  // do the restart
  if (new_game)
    move_to_scene_start(r, Scene::game);
}

} // namespace game2d