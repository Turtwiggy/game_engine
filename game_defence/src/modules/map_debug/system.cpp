#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/map/components.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/spaceship_designer/generation/components.hpp"

namespace game2d {

struct Descriptor
{
  glm::vec2 worldspace_pos = { 0, 0 };
  glm::vec2 size = { 8, 8 };
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

void
update_debug_map_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  const auto map_debug_e = get_first<DebugMapComponent>(r);
  if (map_debug_e == entt::null)
    create_empty<DebugMapComponent>(r);
  auto& map_debug = get_first_component<DebugMapComponent>(r);
  auto& map_debug_pool = map_debug.pool;

  std::vector<Descriptor> request_to_draw_at_position;

  // draw all the wall & floor cells...
  //

  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e != entt::null) {
    const auto& dungeon_c = r.get<DungeonGenerationResults>(dungeon_e);

    for (size_t i = 0; i < map.map.size(); i++) {
      Descriptor d;
      d.size = { map.tilesize, map.tilesize };
      d.worldspace_pos = engine::grid::index_to_world_position_center((int)i, map.xmax, map.ymax, map.tilesize);
      if (dungeon_c.wall_or_floors[i] == 0)
        d.colour = { 1.0f, 1.0f, 1.0f, 0.3f };
      if (dungeon_c.wall_or_floors[i] == 1)
        d.colour = { 1.0f, 0.3f, 0.3f, 0.3f };

      request_to_draw_at_position.push_back(d);
    }
  }

  // debug all grid cells that contain something
  //
  size_t amount_idx = 0;
  for (size_t i = 0; i < map.map.size(); i++) {
    for (const auto e : map.map[i]) {
      Descriptor d;
      d.worldspace_pos = engine::grid::index_to_world_position_center((int)i, map.xmax, map.ymax, map.tilesize);
      d.size = { 8, 8 };
      d.colour = { 1.0f, 0.0f, 0.0f, 0.5f };
      request_to_draw_at_position.push_back(d);
    }
  }

  map_debug_pool.update(r, request_to_draw_at_position.size());
  for (int i = 0; const auto& req : request_to_draw_at_position) {
    const auto instance_e = map_debug_pool.instances[i++];
    set_position(r, instance_e, req.worldspace_pos);
    set_size(r, instance_e, req.size);
    set_colour(r, instance_e, req.colour);
  }
}

} // namespace game2d