#include "ui_spaceship_designer_system.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/map/components.hpp"
#include "modules/map/helpers.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/spaceship_designer/generation/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/spaceship_designer/spaceship_designer_helpers.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"

#include "imgui.h"
#include "modules/ui_inventory/ui_inventory_components.hpp"

namespace game2d {

enum class SpaceshipDesignerMode
{
  PLACE,
};
static SpaceshipDesignerMode mode;

void

update_ui_spaceship_designer_system(entt::registry& r, const glm::vec2& mouse_pos, const float dt)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto generate = cleanup_requests<RequestGenerateDungeonComponent>(r);

  static auto seed = 0;
  static auto rnd = engine::RandomState(seed);
  static auto tilesize = 50.0f;

  ImGui::Begin("Spaceship Designer");

  if (ImGui::Button("(new) empty spaceship")) {
    move_to_scene_start(r, Scene::dungeon_designer);

    destroy_first_and_create<MapComponent>(r);
    auto& map = get_first_component<MapComponent>(r);
    map.tilesize = tilesize;
    map.xmax = 24;
    map.ymax = 24;
    map.map.resize(map.xmax * map.ymax);

    DungeonIntermediate results;
    results.floor_types.resize(map.xmax * map.ymax, FloorType::WALL); // all walls

    // hack: default as a small ship
    // note: this only works for xmax 24 ymax 24+++
    const auto floors = { 25, 26, 49, 50, 73, 74, 97, 98, 121, 122 };
    for (const auto floor_i : floors)
      results.floor_types[floor_i] = FloorType::FLOOR;
    results.floor_types[27] = FloorType::AIRLOCK;

    Room bridge;
    bridge.tiles_idx = { 25, 26 };
    auto bridge_e = create_empty<Room>(r, bridge);
    r.emplace<RoomName>(bridge_e, "Bridge");

    create_empty<DungeonIntermediate>(r, results);
  }

  if (ImGui::Button("(new) random spaceship") || generate) {
    move_to_scene_start(r, Scene::dungeon_designer);

    destroy_first_and_create<MapComponent>(r);
    auto& map = get_first_component<MapComponent>(r);
    map.tilesize = tilesize;
    map.xmax = 24;
    map.ymax = 24;
    map.map.resize(map.xmax * map.ymax);

    DungeonGenerationCriteria spec;
    spec.max_rooms = 10;
    spec.room_size_min = 4;
    spec.room_size_max = glm::min(8, map.xmax);

    auto results = generate_rooms(r, spec, rnd);
    connect_rooms_via_nearest_neighbour(r, results);
    // update_map_with_pathfinding(r, map, results);
    create_empty<DungeonIntermediate>(r, results);
  }

  auto map_e = get_first<MapComponent>(r);
  if (map_e != entt::null && ImGui::Button("Instantiate map")) {
    auto& map_c = r.get<MapComponent>(map_e);
    auto& results_c = get_first_component<DungeonIntermediate>(r);
    {
      convert_tunnels_to_rooms(r, results_c);
      generate_edges(r, map_c, results_c);
      generate_airlocks(r, map_c, results_c);
      instantiate_edges(r, map_c);
      instantiate_floors(r, map_c, results_c);
    }
    destroy_first<DungeonIntermediate>(r); // consider it "processed"

    // create a transform
    auto e = create_transform(r, "ship-small");
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, "ship_small_rusty");
    set_size(r, e, { 200, 300 });
    set_position(r, e, { 100, 150 });
    set_z_index(r, e, ZLayer::BACKGROUND);
  }

  if (ImGui::Button("Add jetpack player")) {
    destroy_first<CameraFreeMove>(r);
    create_jetpack_player(r);
  }

  const auto spawn_mob_impl = [&r](glm::vec2 wp) -> entt::entity {
    const auto mob_e = spawn_mob(r, "dungeon_actor_enemy_default", wp);
    r.emplace<TeamComponent>(mob_e, TeamComponent{ AvailableTeams::enemy });

    auto& inv = r.get<DefaultInventory>(mob_e).inv;
    auto& body = r.get<DefaultBody>(mob_e).body;

    // give the enemy a piece of scrap in their inventory
    spawn_inv_item(r, inv, 0, "scrap");

    // give the enemy a 5% chance to have a medkit in their inventory...
    // TODO: medkits

    // give enemy a weapon
    auto weapon_e = spawn_inv_item(r, body, 6, "shotgun");

    return mob_e;
  };

  if (map_e != entt::null && ImGui::Button("Populate rooms...")) {
    const auto& map_c = r.get<MapComponent>(map_e);
    const auto& view = r.view<Room>();
    for (const auto& [e, room_c] : view.each()) {

      auto idxs = get_empty_slots_in_room(r, map_c, room_c);
      const int n_free_slots = static_cast<int>(idxs.size());
      if (n_free_slots != 0) {
        // choose a random slot...
        const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
        const int slot_idx = idxs[slot_i];
        const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

        // spawn_mob()
        auto mob_e = spawn_mob_impl(pos);
        add_entity_to_map(r, mob_e, slot_idx);

        // spawn_environment(r, slot_idx);

        idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
      }
    }
  }

  if (map_e != entt::null && ImGui::Button("Populate space...")) {
    const auto& map_c = r.get<MapComponent>(map_e);

    auto idxs = get_empty_slots_in_map(r, map_c);

    int n_free_slots = static_cast<int>(idxs.size());
    int amount_to_spawn = 3;

    do {
      // choose a random slot...
      const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
      const int slot_idx = idxs[slot_i];
      const auto pos = engine::grid::index_to_world_position_center(slot_idx, map_c.xmax, map_c.ymax, map_c.tilesize);

      auto mob_e = spawn_mob_impl(pos);

      idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
      amount_to_spawn--;
      n_free_slots--;

    } while (amount_to_spawn > 0 && n_free_slots > 0);

    //
  }

  ImGui::SeparatorText("info: mouse/grid");
  map_e = get_first<MapComponent>(r);
  if (map_e != entt::null) {
    const auto& map_c = r.get<MapComponent>(map_e);
    const auto grid_pos = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
    ImGui::Text("mouse_pos: %f %f", mouse_pos.x, mouse_pos.y);
    ImGui::Text("grid_pos: %i %i", grid_pos.x, grid_pos.y);
    ImGui::Text("grid_idx: %i", engine::grid::grid_position_to_index(grid_pos, map_c.xmax));

    ImGui::SeparatorText("Map");
    const auto mouse_idx = engine::grid::grid_position_to_clamped_index(grid_pos, map_c.xmax, map_c.ymax);
    ImGui::Text("Map at %i is size: %zu", mouse_idx, map_c.map[mouse_idx].size());
    for (const auto map_e : map_c.map[mouse_idx]) {
      if (const auto* tag_c = r.try_get<TagComponent>(map_e))
        ImGui::Text("map_e: %s", tag_c->tag.c_str());
      else
        ImGui::Text("map_e: no tag component?");

      if (const auto* pathfinding_c = r.try_get<PathfindComponent>(map_e)) {
        ImGui::SameLine();
        ImGui::Text("cost: %i", pathfinding_c->cost);
      }
    }

    const auto in_rooms = inside_room(r, grid_pos);
    for (const auto& room : in_rooms)
      ImGui::Text("Inside room: %i", static_cast<uint32_t>(room));
  }

  ImGui::SeparatorText("Rooms");
  ImGui::Text("Rooms: %zu", r.view<Room>().size());
  if (ImGui::Button("Create Room"))
    create_empty<Room>(r);
  // for (const auto& [e, room_c] : rooms.each()) {
  //   ImGui::Text("room...");
  //   for (const auto& idx : room_c.tiles_idx)
  //     ImGui::Text("idx: %i", idx);
  // }

  // debug edges...
  static std::string filter;
  imgui_draw_string("edge_filter", filter);
  ImGui::Text("edges...");
  std::vector<Edge> processed;
  for (const auto& [e, edge_c] : r.view<Edge>().each()) {
    std::string label = std::format("edg: a.x{},a.y{} b.x{},b.y{}", edge_c.a.x, edge_c.a.y, edge_c.b.x, edge_c.b.y);
    auto it = std::find(processed.begin(), processed.end(), edge_c);
    if (it != processed.end())
      label += "[DUPLICATE]";
    processed.push_back(edge_c);
    if (label.find(filter) != std::string::npos)
      ImGui::Text("%s", label.c_str());
  }

  auto dungeon_e = get_first<DungeonIntermediate>(r);
  if (dungeon_e != entt::null) {
    auto& dungeon_c = r.get<DungeonIntermediate>(dungeon_e);
    const auto& map_c = r.get<MapComponent>(map_e);
    ImGui::SeparatorText("WIP dungeon...");

    const auto grid_pos = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
    const auto mouse_idx = engine::grid::grid_position_to_clamped_index(grid_pos, map_c.xmax, map_c.ymax);
    ImGui::Text("wall_or_floor: %i", static_cast<int>(dungeon_c.floor_types[mouse_idx]));

    const auto& input = get_first_component<SINGLE_InputComponent>(r);

    if (get_key_down(input, SDL_SCANCODE_1))
      dungeon_c.floor_types[mouse_idx] = FloorType::WALL;
    if (get_key_down(input, SDL_SCANCODE_2))
      dungeon_c.floor_types[mouse_idx] = FloorType::FLOOR;

    if (get_key_down(input, SDL_SCANCODE_3)) {
      dungeon_c.floor_types[mouse_idx] = FloorType::AIRLOCK;
      // const auto part_e = spawn_ship_part(r, "airlock");
      // const auto pos = engine::grid::index_to_world_position_center(mouse_idx, map_c.xmax, map_c.ymax, map_c.tilesize);
      // set_position(r, part_e, pos);
      // add_entity_to_map(r, part_e, mouse_idx);
    }
    if (get_key_down(input, SDL_SCANCODE_4)) {
      const auto part_e = spawn_ship_part(r, "engine");
      const auto pos = engine::grid::index_to_world_position_center(mouse_idx, map_c.xmax, map_c.ymax, map_c.tilesize);
      set_position(r, part_e, pos);
      add_entity_to_map(r, part_e, mouse_idx);
    }

    static std::set<int> selected_tile_idxs;

    if (get_mouse_lmb_press() && ri.viewport_hovered)
      selected_tile_idxs.emplace(mouse_idx);
    if (get_mouse_rmb_press())
      selected_tile_idxs.clear();

    ImGui::Text("Selected tiles: ");
    for (const auto idx : selected_tile_idxs) {
      ImGui::SameLine();
      ImGui::Text("%i, ", idx);
    }

    //
    for (const auto idx : selected_tile_idxs) {
      const auto gp = engine::grid::index_to_grid_position(idx, map_c.xmax, map_c.ymax);
      const auto in_rooms = inside_room(r, gp);
      ImGui::Text("%i in_rooms: %zu", idx, in_rooms.size());
      for (const auto room_e : in_rooms)
        ImGui::Text("in_room_eid: %i", static_cast<int>(room_e));

      // TODO: convert selected tile in to e.g. airlock
      // const auto& raws = get_first_component<Raws>(r);
      // std::vector<std::string> labels;
      // for (const auto& ship_part : raws.ship_parts)
      //   labels.push_back(ship_part.name);
      // static int select_part_idx = 0;
      // WomboComboIn combo_in(labels);
      // combo_in.label = "SelectShipPart";
      // combo_in.current_index = static_cast<int>(select_part_idx);
      // WomboComboOut combo_out = draw_wombo_combo(combo_in);
      // if (combo_in.current_index != combo_out.selected) {
      //   select_part_idx = combo_out.selected;
      //   // todo: convert the tile...?
      //   SDL_Log("%s", std::format("tile wants to be a: {}", labels[select_part_idx]).c_str());
      // r.get<TagComponent>(e).tag = labels[select_part_idx];
      // }
    }

    // Adds selected tiles to room... note: not currently doing any validation
    for (const auto& [e, room_c] : r.view<Room>().each()) {
      const uint32_t eid = static_cast<uint32_t>(e);
      ImGui::PushID(eid);
      const std::string label = std::format("Add all selected to room");
      if (ImGui::Button(label.c_str()))
        room_c.tiles_idx.insert(room_c.tiles_idx.end(), selected_tile_idxs.begin(), selected_tile_idxs.end());
      ImGui::SameLine();
      ImGui::Text("Room EID: %i, tiles_idx: %zu", eid, room_c.tiles_idx.size());
      ImGui::PopID();
    }
  }

  ImGui::End();
}

} // namespace game2d