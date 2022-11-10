#pragma once

#include "game/components/app.hpp"
#include "game/modules/dungeon/components.hpp"

namespace game2d {

struct WallSpriteComponent
{
  int x = 0;
  int y = 0;
};

glm::ivec2
get_player_grid_position(Game& game);

// THIS IS SLOW
void
set_actors_on_visible_tiles_as_visible(Game& game);

void
update_visible(GameEditor& editor, Game& game);

void
update_hidden(GameEditor& editor, Game& game);

void
update_was_visible(GameEditor& editor, Game& game);

void
work_out_sprite_for_walls(Game& game, const Dungeon& d);

} // namespace game2d