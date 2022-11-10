#include "system.hpp"

#include "components.hpp"
#include "game/components/events.hpp"
#include "game/modules/ai/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "helpers.hpp"
#include "modules/entt/helpers.hpp"

namespace game2d {

void
update_dungeon_system(GameEditor& editor, Game& game, uint32_t& dungeon_seed)
{
  auto& r = game.state;
  {
    const auto& view = r.view<const PlayerComponent, const CollidingWithExitComponent>();
    bool collision_occured = view.size_hint() > 0;
    if (!collision_occured)
      return; // not interested
  }

  // check gameover condition
  auto existing_dungeon = get_first<Dungeon>(r);
  if (r.get<Dungeon>(existing_dungeon).floor == 6) {
    game.ui_events.events.push_back("You win! Thanks for playing.");
    game.gameover = true;
    return;
  }

  transfer_old_state_generate_dungeon(editor, game, dungeon_seed);
};

} // namespace game2d