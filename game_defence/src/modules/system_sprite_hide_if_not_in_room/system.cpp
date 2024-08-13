#include "system.hpp"

#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_sprite_hide_if_not_in_room/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
update_sprite_hide_if_not_in_room_system(entt::registry& r)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  const auto player_pos = get_position(r, player_e);
  const auto player_gridpos = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);
  const auto [player_in_room, player_room] = inside_room(map, dungeon.rooms, player_gridpos);

  const auto& view = r.view<EnemyComponent>();
  for (const auto& [e, room_req] : view.each()) {
    const auto pos = get_position(r, e);
    const auto gridpos = engine::grid::worldspace_to_grid_space(pos, map.tilesize);

    const auto [in_room, room] = inside_room(map, dungeon.rooms, gridpos);

    if (player_in_room) { // player is in any room? i.e. onboard the ship?
      set_sprite(r, e, "PERSON_25_0");
      // r.emplace_or_replace<VisibleComponent>(e);
      // remove_if_exists<HiddenComponent>(r, e);
    } else {
      set_sprite(r, e, "TEXT_?");
      // r.emplace_or_replace<HiddenComponent>(e);
      // remove_if_exists<VisibleComponent>(r, e);
    }
  }

  //
}

} // namespace game2d