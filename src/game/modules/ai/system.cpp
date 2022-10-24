#include "system.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "game/components/events.hpp"
#include "game/modules/dungeon/components.hpp"
#include "game/modules/dungeon/helpers.hpp"
#include "game/modules/player/components.hpp"
#include "helpers.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <map>
#include <utility>
#include <vector>

namespace game2d {

void
update_ai_system(GameEditor& editor, Game& game, const uint64_t& milliseconds_dt)
{
  auto& colours = editor.colours;
  auto& r = game.state;
  static engine::RandomState rnd;

  // hack: only one dungeon at the moment
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const int& x_max = d.width;
  const int GRID_SIZE = 16;

  const auto& player_view = r.view<PlayerComponent>();
  if (player_view.size() == 0)
    return; // no player to home in on

  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  const auto player_grid =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, GRID_SIZE);

  // player position
  vec2i to = { glm::clamp(player_grid.x, 0, d.width - 1), glm::clamp(player_grid.y, 0, d.height - 1) };

  const auto& view = r.view<AiBrainComponent, GridMoveComponent, const TransformComponent>();
  for (auto [entity, ai, move, transform] : view.each()) {

    const auto grid =
      engine::grid::world_space_to_grid_space({ transform.position.x, transform.position.y }, GRID_SIZE);
    vec2i from = { glm::clamp(grid.x, 0, d.width - 1), glm::clamp(grid.y, 0, d.height - 1) };

    ai.milliseconds_between_ai_updates_left -= milliseconds_dt;
    if (ai.milliseconds_between_ai_updates_left <= 0) {
      ai.milliseconds_between_ai_updates_left = k_milliseconds_between_ai_updates;

      // Take an action...

      // If confused... choose a random direction...
      // TODO...

      // Otherwise... try to choose a sane direction...
      const auto path = astar(r, from, to);

      const int max_paths = d.height * d.width;
      if (path.size() > max_paths) // assume something is wrong
        std::cout << "pathfinding has gone wrong\n";

      // next door to entity...
      if (path.size() == 2)
        r.emplace_or_replace<WasCollidedWithComponent>(player_entity);

      // in range of entity...
      if (path.size() > 2 && path.size() < 6) {
        const auto& next_step = path[1]; // path[0] is current
        move.x = ((next_step.x - from.x) * GRID_SIZE);
        move.y = ((next_step.y - from.y) * GRID_SIZE);
      }

      if (path.size() >= 6) {
        // .. choose a random direction
        int rnd_x = static_cast<int>(engine::rand_det_s(rnd.rng, 0, 3)) - 1;
        int rnd_y = static_cast<int>(engine::rand_det_s(rnd.rng, 0, 3)) - 1;
        move.x = rnd_x * GRID_SIZE;
        move.y = rnd_y * GRID_SIZE;
      }

      // debugging
      // for (const auto& p : path) {
      //   int index = x_max * p.y + p.x;
      //   const auto& e = group[index];
      //   auto& col = r.get<SpriteColourComponent>(e);
      //   col.colour = colours.lin_cyan;
      // }
    }
  }
};

} // namespace game2d
