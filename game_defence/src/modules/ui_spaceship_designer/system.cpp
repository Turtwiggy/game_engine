#include "system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/spaceship_designer/helpers.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"

#include "imgui.h"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

// TODO list:
// mess with wall/floor grids like a level editor
// able to add/remove entities on map
// rooms to have types
// rooms generate with stuff in

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::vec2& mouse_pos, const float dt)
{
  const auto& reqs = r.view<RequestGenerateDungeonComponent>();
  const bool generate = reqs.size() > 0;
  r.destroy(reqs.begin(), reqs.end()); // done requests

  ImGui::Begin("Spaceship Designer");

  static engine::RandomState rnd(0);
  static int seed = 0;
  if (imgui_draw_int("seed", seed))
    rnd = engine::RandomState(seed);

  static int tilesize = 50;
  imgui_draw_int("tilesize", tilesize);

  if (ImGui::Button("Generate Rooms (Random)") || generate) {
    move_to_scene_start(r, Scene::dungeon_designer);

    destroy_first_and_create<MapComponent>(r);
    auto& map = get_first_component<MapComponent>(r);
    const int map_width = tilesize * 24;
    const int map_height = tilesize * 24;
    map.tilesize = tilesize;
    map.xmax = map_width / map.tilesize;
    map.ymax = map_height / map.tilesize;
    map.map.resize(map.xmax * map.ymax);

    DungeonGenerationCriteria c;
    c.max_rooms = 10;
    c.room_size_min = 4;
    c.room_size_max = glm::min(8, map.xmax);
    DungeonGenerationResults results = generate_rooms(r, c, rnd);
    connect_rooms_via_nearest_neighbour(r, results);
    // update_map_with_pathfinding(r, map, results);
    generate_edges(r, map, results);
    instantiate_edges(r, map);
    generate_floors(r, map, results);
    create_empty<DungeonGenerationResults>(r, results);

    // set the camera in the center of the newly generated dungeon
    auto camera_e = get_first<OrthographicCamera>(r);
    auto& camera_t = r.get<TransformComponent>(camera_e);
    camera_t.position = { map_width / 2.0f, map_height / 2.0f, 0.0f };
  }

  if (ImGui::Button("Add jetpack player")) {
    destroy_first<CameraFreeMove>(r);

    const auto player_e = create_jetpack_player(r);

    auto results_e = get_first<DungeonGenerationResults>(r);
    if (results_e != entt::null) {
      const auto& results_c = r.get<DungeonGenerationResults>(results_e);

      const auto map_e = get_first<MapComponent>(r);
      if (map_e != entt::null) {
        const auto& map_c = r.get<MapComponent>(map_e);
        // set player_e inside spaceship... in room 0...
        const auto grid_tl = results_c.rooms[0].tl.value();
        const auto pos = engine::grid::grid_space_to_world_space_center(grid_tl, map_c.tilesize);
        set_position(r, player_e, pos);

        add_entity_to_map(r, player_e, engine::grid::grid_position_to_index(grid_tl, map_c.xmax));
      }
    }
  }

  if (ImGui::Button("Populate rooms...")) {
    const auto map_e = get_first<MapComponent>(r);
    if (map_e != entt::null) {
      const auto& map_c = r.get<MapComponent>(map_e);
      const auto& view = r.view<Room>();
      for (const auto& [e, room_c] : view.each()) {

        auto idxs = get_free_slots_idxs(map_c, room_c);
        const int n_free_slots = static_cast<int>(idxs.size());
        if (n_free_slots != 0) {
          // choose a random slot...
          const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
          const int slot_idx = idxs[slot_i];
          const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

          // spawn_mob()
          {
            const auto mob_e = spawn_mob(r, "dungeon_actor_enemy_default", pos);
            r.emplace<TeamComponent>(mob_e, TeamComponent{ AvailableTeams::enemy });

            auto& inv = r.get<DefaultInventory>(mob_e).inv;
            auto& body = r.get<DefaultBody>(mob_e).body;

            // give the enemy a piece of scrap in their inventory
            spawn_inv_item(r, inv, 0, "scrap");

            // give the enemy a 5% chance to have a medkit in their inventory...
            // TODO: medkits

            // give enemy a weapon
            auto weapon_e = spawn_inv_item(r, body, 6, "shotgun");

            add_entity_to_map(r, mob_e, slot_idx);
          }

          // spawn_environment(r, slot_idx);

          idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
        }
      }
    }
  }

  ImGui::SeparatorText("info: mouse/grid");
  const auto map_e = get_first<MapComponent>(r);
  if (map_e != entt::null) {
    const auto& map_c = r.get<MapComponent>(map_e);
    const auto grid_pos = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
    ImGui::Text("mouse_pos: %f %f", mouse_pos.x, mouse_pos.y);
    ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);
    ImGui::Text("grid_idx: %i", engine::grid::grid_position_to_index(grid_pos, map_c.xmax));

    const auto in_rooms = inside_room(r, grid_pos);
    ImGui::Text("In rooms: %i", in_rooms.size() > 0);
    for (const entt::entity room_e : in_rooms)
      ImGui::Text("in_room_e: %i", static_cast<uint32_t>(room_e));

    ImGui::SeparatorText("Map");
    const auto mouse_idx = engine::grid::grid_position_to_clamped_index(grid_pos, map_c.xmax, map_c.ymax);
    ImGui::Text("Map at %i is size: %zu", mouse_idx, map_c.map[mouse_idx].size());
  }

  ImGui::End();
}

} // namespace game2d