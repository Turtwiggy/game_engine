#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/combat/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/helpers.hpp"
#include "modules/hearth/components.hpp"
#include "modules/player/components.hpp"
#include "modules/ui_gameover/components.hpp"

namespace game2d {

void
update_gameover_system(entt::registry& r)
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
  // const auto& first_player = get_first<PlayerComponent>(r);
  // if (first_player != entt::null) {
  //   const auto& health = r.try_get<HealthComponent>(first_player);
  //   if (health && health->hp <= 0) {
  //     gameover.game_is_over = true;
  //     gameover.reason = "Player ded!";
  //   }
  // }

  const auto& view = r.view<NewGameRequest>();
  bool new_game = view.size() > 0;

  // destroy all new game requests
  for (const auto& [entity, req] : view.each())
    r.destroy(entity);

  // do the restart
  if (new_game)
    restart_game(r);

  // HACK: restart game if r is pressed
  const auto& finput = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finput.history.at(finput.fixed_tick);
  bool r_pressed = std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e) {
                     return e.type == InputType::keyboard && e.key == SDL_SCANCODE_R && e.state == InputState::held;
                   }) != std::end(inputs);
  if (r_pressed)
    restart_game(r);
}

} // namespace game2d