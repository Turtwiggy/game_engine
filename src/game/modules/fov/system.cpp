#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "game/components/actors.hpp"
#include "game/modules/ai/components.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/dungeon/components.hpp"
#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/fov/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

#include <vector>

namespace game2d {

std::vector<entt::entity>
grid_entities_at(Game& game, int x, int y)
{
  auto& r = game.state;
  std::vector<entt::entity> results;
  for (const auto [entity, transform] : r.view<const TransformComponent>().each()) {
    glm::ivec2 point = engine::grid::world_space_to_grid_space({ transform.position.x, transform.position.y }, 16);
    if (point.x == x && point.y == y)
      results.push_back(entity);
  }
  return results;
};

void
init_tile_fov_system(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  const auto d = r.view<Dungeon>().front();
  const auto& dungeon = r.get<Dungeon>(d);

  // read-only view of the grid
  const std::vector<StaticDungeonEntity>& group = dungeon.walls_and_floors;

  // hide all walls and floor
  for (const auto& tile : group)
    r.emplace_or_replace<NotVisibleComponent>(tile.entity);
  // hide all actors
  for (const auto [entity, transform] : r.view<TransformComponent>().each())
    r.emplace_or_replace<NotVisibleComponent>(entity);
}

void
update_tile_fov_system(GameEditor& editor, Game& game)
{
  const auto& colours = editor.colours;
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

  // check if any actors are on a visible tile
  for (const auto [entity, visible, transform] : r.view<const VisibleComponent, const TransformComponent>().each()) {
    std::vector<entt::entity> actors = grid_entities_at(game, visible.grid_x, visible.grid_y);
    for (const auto& actor : actors)
      mark_visible(game, actor, visible.grid_x, visible.grid_y);
  }

  //
  // set colours
  //

  // visible
  {
    auto visible_no_hp_view = r.view<const VisibleComponent, SpriteColourComponent, const EntityTypeComponent>();
    visible_no_hp_view.each(
      [&r, &colours](const VisibleComponent& v, SpriteColourComponent& scc, const EntityTypeComponent& et) {
        //
        scc.colour = colours.lin_red;
      });
  }

  // not visible
  {
    auto not_visible_view = r.view<const NotVisibleComponent, SpriteColourComponent>();
    not_visible_view.each([&colours](const NotVisibleComponent& v, SpriteColourComponent& scc) {
      //
      scc.colour = colours.lin_black;
    });
  }

  // was visible
  {
    auto was_visible_view = r.view<const NotVisibleButPreviouslySeenComponent, SpriteColourComponent>();
    was_visible_view.each([&colours](const NotVisibleButPreviouslySeenComponent& v, SpriteColourComponent& scc) {
      //
      scc.colour = colours.lin_feint_white;
    });
  }
}

} // namespace game2d