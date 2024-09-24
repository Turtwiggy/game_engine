#include "modules/combat_show_tiles_in_range/show_tiles_in_range_system.hpp"

#include "actors/helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/entity_pool.hpp"
#include "engine/entt/helpers.hpp"
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
    // ImGui::Text("you: %i %i", grid_pos.x, grid_pos.y);

    const std::vector<glm::ivec2> tiles = generate_accessible_areas(r, map_c, grid_pos, 1);

    for (int i = 0; const auto& tile_gp : tiles) {
      // ImGui::Text("accessible: %i %i", tile_gp.x, tile_gp.y);

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
      const auto anypos = glm::vec2(tile_gp * map_c.tilesize) + offset;
      set_position(r, debug_e, anypos);

      i++;
    }

    break; // only first player
  }
}

} // namespace game2d