#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "game/components/actors.hpp"
#include "game/modules/fov/helpers.hpp"
#include "game/modules/fov/helpers/symmetric_shadowcasting.hpp"
#include "game/modules/player/components.hpp"

namespace game2d {

void
init_tile_fov_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  const auto& colours = editor.colours;
  const auto d = r.view<Dungeon>().front();
  const auto& dungeon = r.get<Dungeon>(d);

  // read-only view of the grid
  const std::vector<StaticDungeonEntity>& group = dungeon.walls_and_floors;

  // hide all walls and floor
  for (const auto& tile : group) {
    EntityTypeComponent& et = r.get<EntityTypeComponent>(tile.entity);
    if (tile.x == 0 || tile.x == dungeon.width - 1 || tile.y == 0 || tile.y == dungeon.height - 1) {
      // Change colours of edge tiles
      auto& scc = r.get<SpriteColourComponent>(tile.entity);
      scc.colour = colours.lin_tertiary; // not visible colour
      continue;
    }
    r.emplace_or_replace<NotVisibleComponent>(tile.entity);
  }

  // hide all actors
  for (const auto [entity, transform, actor] : r.view<TransformComponent, PhysicsActorComponent>().each())
    r.emplace_or_replace<NotVisibleComponent>(entity);

  work_out_sprite_for_walls(game, dungeon);
}

void
update_tile_fov_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  const auto player_view = r.view<PlayerComponent>();
  if (player_view.size() == 0)
    return;
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  glm::ivec2 origin =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, 16);

  // by default, assume everything is not visible anymore
  for (const auto [entity, visible] : r.view<VisibleComponent>().each()) {
    r.remove<VisibleComponent>(entity);
    r.emplace<NotVisibleButPreviouslySeenComponent>(entity);
  }

  do_symmetric_shadowcasting(game, origin);

  set_actors_on_visible_tiles_as_visible(game);

  update_visible(editor, game);
  update_hidden(editor, game);
  update_was_visible(editor, game);
}

} // namespace game2d