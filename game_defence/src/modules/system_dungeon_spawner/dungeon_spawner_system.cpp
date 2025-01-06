#include "dungeon_spawner_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_dungeon_spawner/dungeon_spawner_helpers.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/ui_combat_designer/ui_combat_designer_helpers.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_units/ui_units_components.hpp"
#include "modules/ui_units/ui_units_helpers.hpp"

namespace game2d {

void
update_dungeon_spawner_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(MenuToNextSceneInfo, r, info_e, info_c);
  if (info_c.processed)
    return;
  info_c.processed = true;

  SDL_Log("update_dungeon_spawner_system() spawning units...");

  static int tilesize = 32;
  destroy_first_and_create<MapComponent>(r);
  auto& map = get_first_component<MapComponent>(r);
  map.tilesize = tilesize;
  map.xmax = 10;
  map.ymax = 10;
  map.map.resize(map.xmax * map.ymax);

  auto grid_e = get_first<Effect_GridComponent>(r);
  if (grid_e != entt::null)
    get_first_component<Effect_GridComponent>(r).gridsize = tilesize;

  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);
  const glm::vec2 map_center = { (map_c.xmax * map_c.tilesize) / 2.0f, (map_c.ymax * map_c.tilesize) / 2.0f };
  const glm::vec2 map_size = { map_c.xmax * map_c.tilesize, map_c.ymax * map_c.tilesize };

  const auto units = load_units(r);
  std::vector<UnitType> active_units;
  for (const auto& unit : units) {
    if (unit.active)
      active_units.push_back(unit);
  }
  SDL_Log("Active units: %i", static_cast<int>(active_units.size()));
  auto idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_players(r, idxs, active_units);

  idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_enemies(r, idxs, info_c.level);

  idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_blackhole(r, idxs, info_c.level);

  // center the camera
  const auto camera_e = get_first<OrthographicCamera>(r);
  set_position(r, camera_e, map_center);

  // Set the first unit as the active unit
  auto initiative_group = r.group<InitiativeComponent>();
  initiative_group.sort<InitiativeComponent>(
    [](const InitiativeComponent& a, const InitiativeComponent& b) { return a.initiative < b.initiative; });
  for (const auto& [e, c] : initiative_group.each()) {
    activate_unit(r, e);
    break;
  }

  // create some floor
  // const auto floor_e = spawn_floor(r, "empty", map_center, map_size);
  // set_z_index(r, floor_e, ZLayer::FLOOR);

  static engine::RandomState floor_rnd(0);
  float percent_to_spawn_thing = 100;
  std::vector<glm::ivec2> sprites_sizes{ { 1, 1 }, { 1, 2 }, { 2, 2 } };

  // create some random background things
  for (int idx = 0; idx < map.xmax * map.ymax; idx++) {
    float random_percent = engine::rand_det_s(floor_rnd.rng, 0, 100);
    if (random_percent > percent_to_spawn_thing)
      continue;
    // create something interesting
    const auto pos = engine::grid::index_to_world_position_center(idx, map_c.xmax, map_c.ymax, map_c.tilesize);
    const auto env_e = spawn_environment(r, "decoration", pos);
    const auto rnd_size_idx = engine::rand_det_s(floor_rnd.rng, 0, int(sprites_sizes.size()));
    set_sprite(r, env_e, "EMPTY");
    set_size(r, env_e, sprites_sizes[rnd_size_idx]);
    set_colour(r, env_e, { 1.0, 1.0, 1.0f, 0.1f });
    set_z_index(r, env_e, ZLayer::BACKGROUND);
    r.remove<TeamComponent>(env_e);
  }
}

} // namespace game2d