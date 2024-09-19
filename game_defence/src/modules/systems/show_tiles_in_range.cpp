#include "modules/systems/show_tiles_in_range.hpp"
#include "actors/helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/map/components.hpp"

namespace game2d {

void
update_show_tiles_in_range_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);

  static EntityPool pool;
  pool.update(r, 5); // 1 for player, 4 for surrounding tiles

  const auto& view = r.view<PlayerComponent>();
  for (const auto& [e, player_c] : view.each()) {
    const auto grid_pos = get_grid_position(r, e);
    const auto neighbours = engine::grid::get_neighbour_gridpos(grid_pos, map_c.xmax, map_c.ymax);
    for (int i = 0; const auto& [dir, gpos] : neighbours) {
      const auto debug_e = pool.instances[i];

      // add a transform, if needed
      if (r.try_get<TransformComponent>(debug_e) == nullptr) {
        TransformComponent t_c;
        t_c.scale = { 8, 8, 8 };
        r.emplace<TransformComponent>(debug_e, t_c);
        r.emplace<SpriteComponent>(debug_e);
        set_sprite(r, debug_e, "EMPTY");
      }

      const auto offset = glm::vec2{ map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
      const auto anypos = glm::vec2(gpos * map_c.tilesize) + offset;
      set_position(r, debug_e, anypos);

      i++;
    }

    break; // only first player
  }
}

} // namespace game2d