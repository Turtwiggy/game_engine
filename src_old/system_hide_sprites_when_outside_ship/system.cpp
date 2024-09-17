#include "system.hpp"

#include "actors/bags/core.hpp"
#include "actors/helpers.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_fov/components.hpp"
#include "modules/ux_hoverable/components.hpp"

namespace game2d {

void
update_hide_sprites_when_outside_ship_system(entt::registry& r)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e == entt::null)
    return;
  const auto player_pos = get_position(r, player_e);
  const auto player_gridpos = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);

  const auto rooms = inside_room(r, player_gridpos);
  const bool in_room = rooms.size() > 0;
  const bool outside_ship = !in_room;
  const bool player_in_ship = r.try_get<PlayerInShipComponent>(player_e) != NULL;

  const bool player_moved_inside_ship = !outside_ship && !player_in_ship;
  if (player_moved_inside_ship) {
    fmt::println("player moved inside ship");
    r.emplace_or_replace<PlayerJustMovedIntoShipComponent>(player_e);
  }

  if (outside_ship)
    remove_if_exists<PlayerInShipComponent>(r, player_e);
  else
    r.emplace_or_replace<PlayerInShipComponent>(player_e);

  if (outside_ship) {

    //
    // there's gotta be a better way to do this,
    // but when you're outside the ship, dont show anything in the dungeon
    //

    for (const auto& map_es : map.map)
      for (const auto map_e : map_es) {
        if (const auto* t_c = r.try_get<TransformComponent>(map_e))
          set_size(r, map_e, { 0, 0 });
        if (const auto* has_weapon = r.try_get<HasWeaponComponent>(map_e))
          set_size(r, has_weapon->instance, { 0, 0 });

        else {
          // const auto* tag_c = r.try_get<TagComponent>(map_e);
          // fmt::println("why doesnt: {} have a transform", tag_c->tag);
        }
      }

    return;
  }

  // only inside ship actions now

  if (player_moved_inside_ship) {
    //
    // scale all dungeon entities to their correct size so they're visible
    //
    for (const auto& map_es : map.map) {
      for (const auto map_e : map_es) {
        if (const auto* t_c = r.try_get<TransformComponent>(map_e))
          set_size(r, map_e, DEFAULT_SIZE);
        if (const auto* has_weapon = r.try_get<HasWeaponComponent>(map_e))
          set_size(r, has_weapon->instance, DEFAULT_SIZE);
      }
    }
  }

  // Anything visible
  for (const auto& [e, enemy_c, visible] : r.view<const EnemyComponent, const VisibleComponent>().each()) {
    set_colour(r, e, r.get<DefaultColour>(e).colour);

    // WARNING: set sprites for ALL enemies, reglardless of type...
    set_sprite(r, e, "PERSON_28_1");
  }

  // Seen but not visible
  for (const auto& [e, enemy_c, seen_c] :
       r.view<const EnemyComponent, const SeenComponent>(entt::exclude<VisibleComponent>).each()) {
    set_sprite(r, e, "TEXT_?");
  }

  // Anything not visible (and not seen)
  for (const auto& [e, enemy_c] : r.view<const EnemyComponent>(entt::exclude<VisibleComponent, SeenComponent>).each()) {
    set_sprite(r, e, "TEXT_?");

    // set_size(r, e, { 0, 0 });
    // if (auto* req = r.try_get<HasWeaponComponent>(e))
    //   set_size(r, req->instance, { 0, 0 });
  }
};

} // namespace game2d