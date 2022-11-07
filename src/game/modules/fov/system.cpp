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
      // auto& scc = r.get<SpriteColourComponent>(tile.entity);
      // scc.colour = colours.lin_red;
      continue;
    }
    r.emplace_or_replace<NotVisibleComponent>(tile.entity);
  }
  // hide all actors
  for (const auto [entity, transform, actor] : r.view<TransformComponent, PhysicsActorComponent>().each())
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
    auto visible_no_hp_view =
      r.view<const VisibleComponent, SpriteColourComponent, const EntityTypeComponent, SpriteComponent>();
    visible_no_hp_view.each([&r, &colours, &editor](auto entity,
                                                    const VisibleComponent& v,
                                                    SpriteColourComponent& scc,
                                                    const EntityTypeComponent& et,
                                                    SpriteComponent& sprite) {
      // set as defaults
      const auto col = create_colour(editor, r, entity, et.type);
      scc.colour = col.colour;
      const SpriteComponent spr = create_sprite(editor, r, entity, et.type);
      sprite.x = spr.x;
      sprite.y = spr.y;
    });
  }

  // not visible
  {
    auto not_visible_view = r.view<const NotVisibleComponent, SpriteColourComponent, SpriteComponent>();
    not_visible_view.each(
      [&colours](const NotVisibleComponent& v, SpriteColourComponent& scc, SpriteComponent& sprite) {
        // hidden
        scc.colour = colours.lin_black;
        sprite.x = 0;
        sprite.y = 0;
      });
  }

  // was visible
  {
    auto was_visible_view =
      r.view<const NotVisibleButPreviouslySeenComponent, SpriteColourComponent, SpriteComponent>();
    was_visible_view.each([&r, &colours](auto entity,
                                         const NotVisibleButPreviouslySeenComponent& v,
                                         SpriteColourComponent& scc,
                                         SpriteComponent& sprite) {
      //
      scc.colour = colours.lin_feint_white;

      // make it an obfuscated sprite?
      if (auto* is_actor = r.try_get<PhysicsActorComponent>(entity)) {
        sprite.x = 30;
        sprite.y = 11;
      };
    });
  }
}

} // namespace game2d