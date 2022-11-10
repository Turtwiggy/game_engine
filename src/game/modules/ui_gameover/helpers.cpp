#include "helpers.hpp"

#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/player/components.hpp"

namespace game2d {

void
restart_game(GameEditor& editor, Game& game, uint32_t& seed)
{
  auto& r = game.state;
  for (const auto [entity, player] : r.view<PlayerComponent>().each())
    r.destroy(entity);

  seed = 1;
  std::cout << "restarting game, seed is: " << seed << "\n";

  transfer_old_state_generate_dungeon(editor, game, seed);
  game.gameover = false;
};

} // namespace game2d