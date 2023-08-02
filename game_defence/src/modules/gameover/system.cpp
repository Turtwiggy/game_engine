#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/helpers.hpp"
#include "modules/health/components.hpp"
#include "modules/hearth/components.hpp"
#include "modules/player/components.hpp"
#include "modules/ui_gameover/components.hpp"

namespace game2d {

void
update_gameover_system(entt::registry& r, b2World& world)
{
  auto& gameover = get_first_component<SINGLETON_GameOver>(r);

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
    const auto& first_player_hp = r.get<HealthComponent>(first_player);
    if (first_player_hp.hp <= 0) {
      gameover.game_is_over = true;
      gameover.reason = "Player ded!";
    }
  }

  // process new-game requests
  bool new_game = false;
  const auto& view = r.view<NewGameRequest>();
  for (const auto& [entity, req] : view.each()) {
    r.destroy(entity);
    new_game = true;
  }
  if (new_game)
    restart_game(r, world);

  // HACK: restart game if r is pressed
  const auto& finput = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finput.history.at(finput.fixed_tick);
  bool r_pressed = std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e) {
                     return e.type == InputType::keyboard && e.key == SDL_SCANCODE_R && e.state == InputState::held;
                   }) != std::end(inputs);
  if (r_pressed)
    restart_game(r, world);
}

} // namespace game2d