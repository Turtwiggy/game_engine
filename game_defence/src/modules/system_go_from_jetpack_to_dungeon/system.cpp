#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/system_entered_new_room/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_turnbased/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
update_go_from_jetpack_to_dungeon_system(entt::registry& r)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto dungeon_e = get_first<DungeonGenerationResults>(r);
  if (dungeon_e == entt::null)
    return;
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  const auto& view = r.view<PlayerComponent, MovementJetpackComponent>();
  for (const auto& [e, req_c, jetpack_c] : view.each()) {
    const auto pos = get_position(r, e);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map.tilesize);

    // check if outside ship
    const auto [in_room, room_opt] = inside_room(map, dungeon.rooms, gp);
    const bool in_tunnels = inside_tunnels(dungeon.tunnels, gp).size() > 0;
    if (!in_room && !in_tunnels)
      continue;
    r.remove<MovementJetpackComponent>(e);

    // we're inside a ship now
    dead.dead.emplace(e);
    if (auto* weapon = r.try_get<HasWeaponComponent>(e))
      dead.dead.emplace(weapon->instance);

    // change camera type
    r.remove<CameraFollow>(e);
    create_empty<CameraFreeMove>(r);

    // destroy jetpack player and recreate
    const auto worldspace_pos = get_position(r, e);
    const glm::vec2 worldspace_clamped = engine::grid::worldspace_to_clamped_world_space(worldspace_pos, map.tilesize);
    const glm::vec2 offset = { map.tilesize / 2.0f, map.tilesize / 2.0f };
    CombatEntityDescription desc;
    desc.position = worldspace_clamped + offset;
    desc.team = AvailableTeams::player;
    const auto player_e = create_combat_entity(r, desc);
  }
}

} // namespace game2d