#include "helpers.hpp"

#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/player/components.hpp"

namespace game2d {

void
restart_game(GameEditor& editor, Game& game, const int& seed)
{
  auto& r = game.state;

  game.ui_events.events.clear();

  for (const auto [entity, player] : r.view<PlayerComponent>().each())
    r.destroy(entity);

  std::cout << "restarting game, seed is: " << seed << "\n";
  game.live_dungeon_seed = seed;
  game.live_dungeon_floor = 1;
  transfer_old_state_generate_dungeon(editor, game, game.live_dungeon_seed, game.live_dungeon_floor);
  game.running_state = GameState::START;
};

} // namespace game2d