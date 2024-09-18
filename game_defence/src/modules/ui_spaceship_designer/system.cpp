#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/map/components.hpp"
#include "modules/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/spaceship_designer/helpers.hpp"

#include "imgui.h"
#include "modules/scene/helpers.hpp"
#include "modules/ui_inventory/components.hpp"

namespace game2d {

// TODO list:
// mess with wall/floor grids like a level editor
// able to add/remove entities on map
// rooms to have types
// rooms generate with stuff in

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::vec2& mouse_pos, const float dt)
{
  ImGui::Begin("Spaceship Designer");

  static engine::RandomState rnd(0);
  static int seed = 0;
  if (imgui_draw_int("seed", seed))
    rnd = engine::RandomState(seed);

  static int tilesize = 50;
  imgui_draw_int("tilesize", tilesize);

  // Generate Rooms Randomly
  if (ImGui::Button("Generate Rooms (Random)")) {
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
    auto results = generate_rooms(r, c, rnd);
    connect_rooms_via_nearest_neighbour(r, results);
    update_map_with_pathfinding(r, map, results);
    generate_edges(r, map, results);
    instantiate_edges(r, map);
    generate_floors(r, map, results);
    create_empty<DungeonGenerationResults>(r, results);

    // set the camera in the center of the newly generated dungeon
    auto camera_e = get_first<OrthographicCamera>(r);
    auto& camera_t = r.get<TransformComponent>(camera_e);
    camera_t.position = { map_width / 2.0f, map_height / 2.0f, 0.0f };
  }

  // TODO: replace this with generic item/mob spawner
  if (ImGui::Button("Add player")) {
    destroy_first<CameraFreeMove>(r);

    // create_player();
    auto e = spawn_mob(r, "dungeon_actor_hero", { 0, 0 });
    r.emplace<CameraFollow>(e);
    r.emplace<CircleComponent>(e);
    r.emplace<PlayerComponent>(e);
    r.emplace<TeamComponent>(e, AvailableTeams::player);
    r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;
    r.emplace<MovementJetpackComponent>(e);
    r.emplace<DefaultBody>(e, DefaultBody(r));
    r.emplace<DefaultInventory>(e, DefaultInventory(r, 6 * 5));
    r.emplace<InitBodyAndInventory>(e);
    spawn_particle_emitter(r, "anything", { 0, 0 }, e);

    // give helmet to breathe
    // const auto helmet_e = create_transform(r);
    // set_sprite(r, helmet_e, "HELMET_5");
    // set_size(r, helmet_e, get_size(r, e));
    // set_z_index(r, helmet_e, ZLayer::PLAYER_HELMET);
    // set_colour(r, helmet_e, { 1.0f, 1.0f, 1.0f, 1.0f });
    // r.emplace<DynamicTargetComponent>(helmet_e).target = e;
    // r.emplace<SetRotationAsDynamicTarget>(helmet_e, SetRotationAsDynamicTarget{ 6.0f });
    // r.remove<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  }

  if (ImGui::Button("Populate room...")) {

    /*
    #void
    spawn_enemy(entt::registry& r, const int idx)
    {
      const auto& map_c = get_first_component<MapComponent>(r);

      DataDungeonActor desc;
      desc.pos = engine::grid::index_to_world_position_center(idx, map_c.xmax, map_c.ymax, map_c.tilesize);
      desc.team = AvailableTeams::enemy;
      desc.hp = 50;
      desc.max_hp = 50;
      desc.colour = engine::SRGBColour{ 0.9f, 0.9f, 0.9f, 1.0f };
      const auto dungeon_e = Factory_DataDungeonActor::create(r, desc);

      // give the enemy a piece of scrap in their inventory
      create_inv_scrap(r, r.get<DefaultInventory>(dungeon_e).inv[0]);

      // give the enemy a 5% chance to have a medkit in their inventory...
      // TODO: medkits

      // get enemies to their inventory
      r.emplace<DropItemsOnDeathComponent>(dungeon_e);

      // give enemies a shotgun
      DataWeaponShotgun wdesc;
      wdesc.able_to_shoot = true;
      wdesc.parent = dungeon_e;
      wdesc.team = desc.team;
      const auto weapon_e = Factory_DataWeaponShotgun::create(r, wdesc);

      // link entity&weapon
      HasWeaponComponent has_weapon_c;
      has_weapon_c.instance = weapon_e;
      r.emplace<HasWeaponComponent>(dungeon_e, has_weapon_c);
    };

    void
    spawn_cover(entt::registry& r, const int idx)
    {
      const auto& map_c = get_first_component<MapComponent>(r);

      DataDungeonCover desc;
      desc.pos = engine::grid::index_to_world_position_center(idx, map_c.xmax, map_c.ymax, map_c.tilesize);
      // fmt::println("cover generated: {},{}", desc.pos.x, desc.pos.y);

      const auto e = Factory_DataDungeonCover::create(r, desc);
    };
    */

    /*
    void spawn_cover_in_free_slots(entt::registry & r, FreeSlots & free_slots, int amount)
    {
      for (int i = amount; i > 0; i--) {
        const int n_free_slots = static_cast<int>(free_slots.size());
        if (n_free_slots == 0)
          return;

        // choose a random free slot
        const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
        const int slot_idx = free_slots[slot_i];

        spawn_cover(r, slot_idx);

        // remove slot from free slot
        free_slots.erase(free_slots.begin() + slot_i);
      }
    };
    */

    /*
    void
    set_generated_entity_positions(entt::registry& r, DungeonGenerationResults& results, engine::RandomState& rnd)
    {
      const auto& map_c = get_first_component<MapComponent>(r);

      std::vector<std::pair<Room, FreeSlots>> room_to_free_slots;

      // i = 0 is the player room
      for (size_t i = 1; i < results.rooms.size(); i++) {
        const Room& room = results.rooms[i];
        const auto free_slots = get_free_slots_idxs(map_c, room);
        room_to_free_slots.push_back({ room, free_slots });
      }

      // Generate some things
      for (auto& [room, free_slots] : room_to_free_slots) {
        spawn_enemy_in_free_slots(r, free_slots, 3);
        spawn_cover_in_free_slots(r, free_slots, 1);
      }

      //
    };
    */
  }

  ImGui::End();
}

} // namespace game2d