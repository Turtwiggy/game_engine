#include "helpers.hpp"

#include "maths/grid.hpp"

namespace game2d {

void
light_up_room(std::vector<Light>& lights,
              int& i,
              const Room& room,
              const SINGLETON_RendererInfo& ri,
              const TransformComponent& camera_t)
{
  const int room_size = room.aabb.size.x * room.aabb.size.y;

  float lum = 0.85f;

  // less bright in smaller rooms
  if (room_size <= 16)
    lum = 0.6f;

  // Orange light top left
  {
    Light& l = lights[i++];
    l.enabled = true;

    // gridspace to worldspace
    const glm::ivec2 gridpos = room.tl;
    l.pos = engine::grid::grid_space_to_world_space_center(gridpos, 50);

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

    // gridspace to worldspace
    const glm::ivec2 gridpos = { room.tl.x + room.aabb.size.x - 1, room.tl.y + room.aabb.size.y - 1 };
    l.pos = engine::grid::grid_space_to_world_space_center(gridpos, 50);

    // worldspace to screenspace
    const auto& wh = ri.viewport_size_render_at;
    l.pos -= glm::vec2{ camera_t.position.x, camera_t.position.y };
    l.pos += glm::vec2{ wh.x / 2.0f, wh.y / 2.0f };

    l.colour = engine::SRGBColour{ 0.6f, 0.6f, 1.0f, 1.0f };
    l.luminence = lum;
  }
}

} // namespace game2d