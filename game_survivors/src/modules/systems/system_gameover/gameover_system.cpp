#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "gameover_system.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/systems/system_gameover/gameover_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_stats/stats_components.hpp"
#include "modules/ui/ui_gameover/ui_gameover_components.hpp"
#include "modules/ui/ui_scene_survive_timer/ui_survive_timer_components.hpp"

namespace game2d {

void
update_gameover_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto game_over_view = r.view<GameOverComponent>();
  if (game_over_view.size() > 0)
    return; // game already ended

  // note: having system rely on ui state is pretty sus
  const auto ui_gameover_e = get_first<SINGLE_GameoverUI>(r);
  if (ui_gameover_e == entt::null)
    return;
  const auto game_over_ui_open = r.get<SINGLE_GameoverUI>(ui_gameover_e).open;
  if (game_over_ui_open)
    return; // game already ended

  GET_FIRST_OR_RETURN(SINGLE_GoldComponent, r, gold_e, gold_c);
  GET_FIRST_OR_RETURN(SINGLE_SurviveStatsComponent, r, stats_e, stats_c);

  //
  // win condition: survive the timer
  //
  for (const auto& [e, timer_c] : r.view<const SurviveTimerComponent>().each()) {

    const int seconds = static_cast<int>(timer_c.time_left_cur) % 60;
    const int minutes = static_cast<int>(timer_c.time_left_cur) / 60;

    const int cu_minutes = (timer_c.time_left_max / 60) - 1 - minutes;
    const int cu_seconds = 59 - seconds;
    // ImGui::Text("%i %i", cu_minutes, cu_seconds);

    if (cu_minutes >= 9 && cu_seconds >= 59) {
      // you win!
      GameOverComponent gameover_c;
      gameover_c.win_condition = true;
      gameover_c.reason = "You survived";
      create_empty<GameOverComponent>(r, gameover_c);

      // update stats
      stats_c.gold_earned = gold_c.temp_amount_pickup + gold_c.temp_amount_enemies;

      // Save collected gold to disk.
      gold_c.amount += stats_c.gold_earned;
      gold_c.temp_amount_pickup = 0;
      gold_c.temp_amount_enemies = 0;
      savefile_put_key(r, "GOLD_AMOUNT", gold_c.amount);
      savefile_save_disk(r);
      SDL_Log("Saved new gold to disk... %i", gold_c.amount);
    }
  }

  //
  // lose condition: all players dead
  //
  if (r.view<PlayerBoatComponent>().size() == 0) {
    GameOverComponent gameover_c;
    gameover_c.win_condition = false;
    gameover_c.reason = "All players dead";
    create_empty<GameOverComponent>(r, gameover_c);

    // update stats
    stats_c.gold_earned = gold_c.temp_amount_pickup + gold_c.temp_amount_enemies;

    // Save collected gold to disk.
    // Even if you lose. too harsh otherwise?
    gold_c.amount += stats_c.gold_earned;
    gold_c.temp_amount_pickup = 0;
    gold_c.temp_amount_enemies = 0;
    savefile_put_key(r, "GOLD_AMOUNT", gold_c.amount);
    savefile_save_disk(r);
    SDL_Log("Saved new gold to disk... %i", gold_c.amount);
  }
}

} // namespace game2d