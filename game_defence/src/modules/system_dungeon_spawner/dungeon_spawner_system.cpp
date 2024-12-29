#include "dungeon_spawner_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/map/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/system_dungeon_spawner/dungeon_spawner_helpers.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/ui_combat_designer/ui_combat_designer_helpers.hpp"
#include "modules/ui_scene_main_menu/components.hpp"

namespace game2d {

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

  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);
  const glm::vec2 map_center = { (map_c.xmax * map_c.tilesize) / 2.0f, (map_c.ymax * map_c.tilesize) / 2.0f };
  const glm::vec2 map_size = { map_c.xmax * map_c.tilesize, map_c.ymax * map_c.tilesize };

  auto idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_enemies(r, idxs, info_c.level);

  idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_blackhole(r, idxs, info_c.level);

  idxs = get_empty_slots_in_map(r, map_c);
  spawn_n_players(r, idxs, 1);

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

  const auto floor_e = spawn_floor(r, "empty", map_center, map_size);
  set_z_index(r, floor_e, ZLayer::FLOOR);
}

} // namespace game2d