#include "gen_players.hpp"

#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"

namespace game2d {

void
set_player_positions(entt::registry& r, const DungeonGenerationResults& results, engine::RandomState& rnd)
{
  const auto& map_c = get_first_component<MapComponent>(r);

  for (const auto& room : results.rooms) {
    const glm::ivec2 tl = room.tl;
    const glm::ivec2 br = room.tl + glm::ivec2{ room.aabb.size.x, room.aabb.size.y };
    const glm::ivec2 worldspace_center = engine::grid::grid_space_to_world_space(room.aabb.center, map_c.tilesize);

    // set camera on this room center
    const auto& camera_e = get_first<OrthographicCamera>(r);
    auto& camera_t = r.get<TransformComponent>(camera_e);
    camera_t.position = { worldspace_center.x, worldspace_center.y, 0.0 };

    // place players in first room
    for (int i = 0; i < 1; i++) {
      const int x = engine::rand_det_s(rnd.rng, tl.x + 1, br.x - 1);
      const int y = engine::rand_det_s(rnd.rng, tl.y + 1, br.y - 1);
      const glm::ivec2 grid_index = { x, y };
      const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(grid_index, map_c.tilesize);
      const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
      const glm::ivec2 pos = worldspace + offset;

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::player;
      const auto e = create_combat_entity(r, desc);
    }

    break; // only spawn players in first room
  }
};

} // namespace game2d