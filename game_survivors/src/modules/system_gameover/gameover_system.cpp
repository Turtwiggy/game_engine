#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/io/settings.hpp"
#include "gameover_components.hpp"
#include "gameover_system.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_item_gold/gold_components.hpp"
#include "modules/ui_scene_survive_timer/ui_survive_timer_components.hpp"

namespace game2d {

void
update_gameover_system(entt::registry& r)
{
  auto game_over_view = r.view<GameOverComponent>();
  if (game_over_view.size() > 0)
    return; // game already ended

  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);

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

      // Save collected gold to disk.
      gold_c.amount += gold_c.temp_amount;
      gold_c.temp_amount = 0;
      auto new_amount = std::to_string(gold_c.amount);
      save_string("GOLD_AMOUNT", new_amount);

      SDL_Log("Saved new gold to disk... %s", new_amount.c_str());
    }
  }

  const auto& view = r.view<PlayerComponent>();
  if (view.size() == 0) {
    // you lose!
    GameOverComponent gameover_c;
    gameover_c.win_condition = false;
    gameover_c.reason = "All players dead";
    create_empty<GameOverComponent>(r, gameover_c);
  }
}

} // namespace game2d