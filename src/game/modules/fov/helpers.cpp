#include "helpers.hpp"

#include "engine/maths/grid.hpp"
#include "game/components/actors.hpp"
#include "game/modules/ai/helpers.hpp"
#include "game/modules/combat/components.hpp"
#include "game/modules/dungeon/components.hpp"
#include "game/modules/fov/components.hpp"
#include "game/modules/fov/helpers/symmetric_shadowcasting.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

void
work_out_sprite_for_walls(Game& game, const Dungeon& d)
{
  auto& r = game.state;

  const auto& view = r.view<EntityTypeComponent>();
  for (auto sde : d.walls_and_floors) {
    EntityType& type = r.get<EntityTypeComponent>(sde.entity).type;
    if (type != EntityType::tile_type_wall)
      continue; // only interested in walls;

    WallSpriteComponent& wall_sprite = r.emplace<WallSpriteComponent>(sde.entity);
    wall_sprite.x = 0;
    wall_sprite.y = 0;

    int wall_bit = 0;
    {
      std::vector<std::pair<GridDirection, int>> udlr_results;
      get_neighbour_indicies(sde.x, sde.y, d.width, d.height, udlr_results);
      for (int i = 0; i < udlr_results.size(); i++) {
        int index = udlr_results[i].second;
        StaticDungeonEntity se = d.walls_and_floors[index];
        EntityTypeComponent e = r.get<EntityTypeComponent>(se.entity);
        if (e.type == EntityType::tile_type_wall)
          wall_bit |= static_cast<int>(udlr_results[i].first);
      }
    }

    int floor_bit = 0;
    {
      std::vector<std::pair<GridDirection, int>> all_results;
      get_neighbour_indicies_with_diagonals(sde.x, sde.y, d.width, d.height, all_results);
      for (int i = 0; i < all_results.size(); i++) {
        int index = all_results[i].second;
        StaticDungeonEntity se = d.walls_and_floors[index];
        EntityTypeComponent e = r.get<EntityTypeComponent>(se.entity);
        if (e.type == EntityType::tile_type_floor)
          floor_bit |= static_cast<int>(all_results[i].first);
      }
    }

    // 1s: no sprites match this case perfectly :(
    if (wall_bit == 1) { // N
      wall_sprite.x = 0;
      wall_sprite.y = 15;
    } else if (wall_bit == 2) { // S
      wall_sprite.x = 0;
      wall_sprite.y = 15;
    } else if (wall_bit == 4) { // E
      wall_sprite.x = 0;
      wall_sprite.y = 15;
    } else if (wall_bit == 8) { // W
      wall_sprite.x = 0;
      wall_sprite.y = 15;
    }
    // 2s: ugly cases due to no sprite matching these either
    else if (wall_bit == 3) { // NS
      wall_sprite.x = 0;
      wall_sprite.y = 15;
    } else if (wall_bit == 12) { // EW
      wall_sprite.x = 0;
      wall_sprite.y = 15;
    }
    // 2s: pretty cases
    else if (wall_bit == 5) { // NE
      wall_sprite.x = 18;
      wall_sprite.y = 2;
    } else if (wall_bit == 6) { // SE
      wall_sprite.x = 18;
      wall_sprite.y = 0;
    } else if (wall_bit == 9) { // NW
      wall_sprite.x = 20;
      wall_sprite.y = 2;
    } else if (wall_bit == 10) { // SW
      wall_sprite.x = 20;
      wall_sprite.y = 0;
    }
    // 4s
    else if (wall_bit == 15) { // all four walls connected
      if (floor_bit == 0) {    // no floors!
        wall_sprite.x = 0;
        wall_sprite.y = 0;
      } else if (floor_bit == 5) { // tl or ne
        wall_sprite.x = 18;
        wall_sprite.y = 4;
      } else if (floor_bit == 9) { // tr or nw
        wall_sprite.x = 19;
        wall_sprite.y = 4;
      } else if (floor_bit == 6) { // bl or se
        wall_sprite.x = 18;
        wall_sprite.y = 3;
      } else if (floor_bit == 10) { // br or sw
        wall_sprite.x = 19;
        wall_sprite.y = 3;
      }
    }
    // 3s
    else if (wall_bit == 7) { // nse
      wall_sprite.x = 18;
      wall_sprite.y = 1;
    } else if (wall_bit == 11) { // nsw
      wall_sprite.x = 20;
      wall_sprite.y = 1;
    } else if (wall_bit == 13) { // new
      wall_sprite.x = 19;
      wall_sprite.y = 2;
    } else if (wall_bit == 14) { // sew
      wall_sprite.x = 19;
      wall_sprite.y = 0;
    }

    if (wall_sprite.x == 0 && wall_sprite.y == 0 && floor_bit != 0) {
      std::cout << "missing wallbit: " << wall_bit << " floorbit: " << floor_bit << "\n";
    }

    SpriteComponent& sprite = r.get<SpriteComponent>(sde.entity);
    sprite.x = wall_sprite.x;
    sprite.y = wall_sprite.y;
  }
};

std::vector<entt::entity>
grid_entities_at(Game& game, int x, int y)
{
  auto& r = game.state;
  std::vector<entt::entity> results;
  for (const auto [entity, actor, transform] : r.view<const PhysicsActorComponent, const TransformComponent>().each()) {
    glm::ivec2 point = engine::grid::world_space_to_grid_space({ transform.position.x, transform.position.y }, 16);
    if (point.x == x && point.y == y)
      results.push_back(entity);
  }
  return results;
};

// THIS IS SLOW
void
set_actors_on_visible_tiles_as_visible(Game& game)
{
  auto& r = game.state;
  for (const auto [entity, visible, transform] : r.view<const VisibleComponent, const TransformComponent>().each()) {
    std::vector<entt::entity> actors = grid_entities_at(game, visible.grid_x, visible.grid_y);
    for (const auto& actor : actors)
      mark_visible(game, actor, visible.grid_x, visible.grid_y);
  }
}

void
update_visible(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto& colours = editor.colours;

  auto view = r.view<SpriteColourComponent, SpriteComponent, const VisibleComponent, const EntityTypeComponent>();
  for (const auto [entity, scc, sc, visible, et] : view.each()) {

    // skip if flashing
    if (auto* flashing = r.try_get<FlashSpriteComponent>(entity))
      continue;

    //
    // set as defaults
    const auto col = create_colour(editor, et.type);
    // scc.colour = engine::SRGBToLinear(colours.secondary);
    scc.colour = col.colour;

    // replaces the sprite with the default sprite
    if (et.type != EntityType::tile_type_wall) {
      const SpriteComponent spr = create_sprite(editor, et.type);
      sc.x = spr.x;
      sc.y = spr.y;
    } else {
      // precalculated because walls are dynamically set
      const WallSpriteComponent& spr = r.get<WallSpriteComponent>(entity);
      sc.x = spr.x;
      sc.y = spr.y;
    }
  }
};

void
update_hidden(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto& colours = editor.colours;

  auto not_visible_view = r.view<const NotVisibleComponent, SpriteColourComponent, SpriteComponent>();
  not_visible_view.each([&colours](const NotVisibleComponent& v, SpriteColourComponent& scc, SpriteComponent& sprite) {
    // hidden
    scc.colour = colours.lin_background;
    sprite.x = 0;
    sprite.y = 0;
  });
};

void
update_was_visible(GameEditor& editor, Game& game)
{
  auto& r = game.state;
  auto& colours = editor.colours;

  auto was_visible_view = r.view<const NotVisibleButPreviouslySeenComponent, SpriteColourComponent, SpriteComponent>();
  was_visible_view.each(
    [&r, &colours](
      auto entity, const NotVisibleButPreviouslySeenComponent& v, SpriteColourComponent& scc, SpriteComponent& sprite) {
      //
      scc.colour = engine::SRGBToLinear(colours.tertiary);

      // make it an obfuscated sprite?
      if (auto* is_actor = r.try_get<PhysicsActorComponent>(entity)) {
        sprite.x = 30;
        sprite.y = 11;
      };
    });
};

} // namespace game2d