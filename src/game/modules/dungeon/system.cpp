#include "system.hpp"

#include "components.hpp"
#include "game/components/events.hpp"
#include "game/modules/ai/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "helpers.hpp"
#include "modules/entt/helpers.hpp"

namespace game2d {

void
update_dungeon_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  if (r.view<PlayerComponent>().size() == 0) {
    game.running_state = GameState::GAMEOVER_LOSE;
    return;
  }

  {
    const auto& view = r.view<const PlayerComponent, const CollidingWithExitComponent>();
    bool collision_occured = view.size_hint() > 0;
    if (!collision_occured)
      return; // not interested
  }

  // check gameover condition
  auto existing_dungeon = get_first<Dungeon>(r);
  if (r.get<Dungeon>(existing_dungeon).floor == 6) {
    game.running_state = GameState::GAMEOVER_WIN;
    return;
  }

  // The game needs to generate a new dungeon, increase the seed!
  game.live_dungeon_seed += 1;
  game.live_dungeon_floor += 1;
  transfer_old_state_generate_dungeon(editor, game, game.live_dungeon_seed, game.live_dungeon_floor);
};

} // namespace game2d