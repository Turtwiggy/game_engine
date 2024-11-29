#include "helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

/*
void
light_up_room(entt::registry& r, std::vector<Light>& lights, int& i, const Room& room, const TransformComponent& camera_t)
{
  // const int room_size = room.aabb.size.x * room.aabb.size.y;
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& map = get_first_component<MapComponent>(r);

  float lum = 0.85f;

  // less bright in smaller rooms
  // if (room_size <= 16)
  //   lum = 0.6f;

  // Orange light top left
  {
    Light& l = lights[i++];
    l.enabled = true;

    // room's first cell
    // index to worldspace
    l.pos = engine::grid::index_to_world_position_center(room.tiles_idx[0], map.xmax, map.ymax, map.tilesize);

    // worldspace to screenspace
    const auto& wh = ri.viewport_size_render_at;
    l.pos -= glm::vec2{ camera_t.position.x, camera_t.position.y };
    l.pos += glm::vec2{ wh.x / 2.0f, wh.y / 2.0f };

    // orange
    l.colour = engine::SRGBColour{ 1.0f, 0.75f, 0.5f, 1.0f };
    l.luminence = lum;
  }

  // A light in bottom right
  {
    Light& l = lights[i++];
    l.enabled = true;

    // room's last cell
    // index to worldspace
    l.pos = engine::grid::index_to_world_position_center(
      room.tiles_idx[room.tiles_idx.size() - 1], map.xmax, map.ymax, map.tilesize);

    // worldspace to screenspace
    const auto& wh = ri.viewport_size_render_at;
    l.pos -= glm::vec2{ camera_t.position.x, camera_t.position.y };
    l.pos += glm::vec2{ wh.x / 2.0f, wh.y / 2.0f };

    l.colour = engine::SRGBColour{ 0.6f, 0.6f, 1.0f, 1.0f };
    l.luminence = lum;
  }
}
*/

} // namespace game2d