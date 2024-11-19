#include "dungeon_spawner_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/physics/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_dungeon_spawner/dungeon_spawner_helpers.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"

namespace game2d {

static auto seed = 0;
static auto player_rnd = engine::RandomState(seed);

void
update_dungeon_spawner_system(entt::registry& r)
{
  static int tilesize = 50;

  const auto info_e = get_first<MenuToNextSceneInfo>(r);
  if (info_e == entt::null)
    return;
  auto& info_c = r.get<MenuToNextSceneInfo>(info_e);

  if (info_c.processed)
    return;
  info_c.processed = true;

  destroy_first_and_create<MapComponent>(r);
  auto& map = get_first_component<MapComponent>(r);
  map.tilesize = tilesize;
  map.xmax = 10;
  map.ymax = 10;
  map.map.resize(map.xmax * map.ymax);

  // spawn the right amount of enemies...
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);
  auto idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_enemies(r, idxs, info_c.level);

  // spawn some kill tiles
  idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_blackhole(r, idxs, info_c.level);

  // spawn the right amount of players...
  for (int i = 0; i < 1; i++) {
    auto idxs = get_empty_slots_in_map(r, map_c);
    const int slot_idx = idxs[engine::rand_det_s(player_rnd.rng, 0, idxs.size())];
    const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

    auto e = spawn_mob(r, "dungeon_actor_hero", pos);
    // r.emplace<CircleComponent>(e);
    r.emplace<PlayerComponent>(e);
    r.emplace<TeamComponent>(e, AvailableTeams::player);
    r.emplace<InitBodyAndInventory>(e);
    // spawn_particle_emitter(r, "anything", pos, e);
    add_entity_to_map(r, e, slot_idx);
  }

  // center the camera
  const auto camera_e = get_first<OrthographicCamera>(r);
  set_position(r, camera_e, { (map_c.xmax * map_c.tilesize) / 2.0f, (map_c.ymax * map_c.tilesize) / 2.0f });
}

} // namespace game2d