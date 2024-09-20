#include "system.hpp"

#include "actors/helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/map/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"
#include "modules/spaceship_designer/helpers.hpp"

#include "imgui.h"
#include "modules/scene/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
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

  // Generate Rooms Randomly
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
    const auto helmet_e = create_transform(r, "player helmet");
    r.emplace<SpriteComponent>(helmet_e);
    set_sprite(r, helmet_e, "HELMET_5");
    set_size(r, helmet_e, get_size(r, e));
    set_z_index(r, helmet_e, ZLayer::PLAYER_HELMET);
    set_colour(r, helmet_e, { 1.0f, 1.0f, 1.0f, 1.0f });
    r.emplace<DynamicTargetComponent>(helmet_e).target = e;
    r.emplace<SetRotationAsDynamicTarget>(helmet_e, SetRotationAsDynamicTarget{ 6.0f });
    r.emplace<TeamComponent>(helmet_e, r.get<TeamComponent>(e));
  }

  if (ImGui::Button("Populate rooms...")) {
    const auto map_e = get_first<MapComponent>(r);
    if (map_e != entt::null) {
      const auto& map_c = r.get<MapComponent>(map_e);
      const auto& view = r.view<Room>();
      for (const auto& [e, room_c] : view.each()) {

        auto idxs = get_free_slots_idxs(map_c, room_c);
        const int n_free_slots = static_cast<int>(idxs.size());
        if (n_free_slots == 0)
          return;
        const int slot_i = engine::rand_det_s(rnd.rng, 0, n_free_slots);
        const int slot_idx = idxs[slot_i];

        // spawn_environment(r, slot_idx);

        idxs.erase(idxs.begin() + slot_i); // remove slot from free slot
      }
    }
  }

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

// auto enemy_e = spawn_mob(r, "dungeon_actor_enemy_default");
// r.emplace<TeamComponnent>(enemylymyymly);
// give the enemy a piece of scrap in their inventory
// create_inv_scrap(r, r.get<DefaultInventory>(dungeon_e).inv[0]);
// give the enemy a 5% chance to have a medkit in their inventory...
// TODO: medkits
// r.emplace<DropItemsOnDeathComponent>(dungeon_e);
// give enemies a shotgun
// DataWeaponShotgun wdesc;
// wdesc.able_to_shoot = true;
// wdesc.parent = dungeon_e;
// wdesc.team = desc.team;
// const auto weapon_e = Factory_DataWeaponShotgun::create(r, wdesc);
// link entity&weapon
// HasWeaponComponent has_weapon_c;
// has_weapon_c.instance = weapon_e;
// r.emplace<HasWeaponComponent>(dungeon_e, has_weapon_c);

// add_cover()
// const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
// PhysicsDescription pdesc;
// pdesc.type = b2_staticBody;
// pdesc.is_bullet = false;
// pdesc.density = 1.0;
// pdesc.position = desc.pos;
// pdesc.size = desc.size;
// pdesc.is_sensor = true;
// create_physics_actor(r, e, pdesc);
// r.emplace<PathfindComponent>(e, -1); // cant pass through
// r.emplace<CoverComponent>(e);
// auto col = get_srgb_colour_by_tag(r, "solid_wall");
// r.emplace<DefaultColour>(e, col);
// set_colour(r, e, col);
// r.emplace<DefenceHitListComponent>(e);
// const auto& map_c = get_first_component<MapComponent>(r);
// add_entity_to_map(r, e, engine::grid::worldspace_to_index(desc.pos, map_c.tilesize, map_c.xmax, map_c.ymax));

// add_breachcharge()
// auto e = spawn_item("breach charge");
// spawn_item_in_world(e);
// r.emplace<EntityTimedLifecycle>(e, 3 * 1000);
// r.emplace<BreachChargeComponent>(e);
// add_bomb_callback(r, e);

// add_lootbag()
// auto e = spawn_item(r, "lootbag");
// r.emplace<LootbagComponent>(e);
// r.emplace<DefaultInventory>(e, desc.inventory);
// r.emplace<PathfindComponent>(e, PathfindComponent{ 0 }); // free to move through
// set_z_index(r, e, ZLayer::BEHIND_PLAYER);
// const auto& map_c = get_first_component<MapComponent>(r);
// add_entity_to_map(r, e, engine::grid::worldspace_to_index(desc.pos, map_c.tilesize, map_c.xmax, map_c.ymax));

// spawn_dungeonactor()
// r.emplace<TurnBasedUnitComponent>(e);
// r.emplace<SpawnParticlesOnDeath>(e);
// r.emplace<HealthComponent>(e, desc.hp, desc.max_hp);
// r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
// r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
// r.emplace<TeamComponent>(e, desc.team);
// r.emplace<DestroyBulletOnCollison>(e);
// r.emplace<DefaultColour>(e, desc.colour);
// add_entity_to_map(r, e, idx);
// OnDeathCallback callback;
// callback.callback = [](entt::registry& r, const entt::entity e) {
//   //
//   drop_inventory_on_death_callback(r, e);
// };
// r.emplace<OnDeathCallback>(e, callback);

// create_cargo()
// auto e = spawn_item(r, "cargo");
// add_item_to_world(e);

// create_bullet()
// const auto e = Factory_BaseActor::create(r, desc, typeid(desc).name());
// const auto size = SMALL_SIZE;
// const bool is_bullet = true;
// create_physics_actor_dynamic(r, e, desc.pos, size, is_bullet);
// auto& t = r.get<TransformComponent>(e);
// t.rotation_radians.z = desc.rotation;
// t.scale = { size.x, size.y, 0.0f };
// const b2Vec2 vel = { desc.dir.x * desc.bullet_speed, desc.dir.y * desc.bullet_speed };
// auto& bullet_c = r.get<PhysicsBodyComponent>(e);
// bullet_c.base_speed = desc.bullet_speed;
// bullet_c.body->SetLinearVelocity(vel);
// r.emplace<AttackComponent>(e, AttackComponent{ desc.bullet_damage });
// r.emplace<AttackIdComponent>(e);
// r.emplace<EntityTimedLifecycle>(e);
// r.emplace<HasParentComponent>(e).parent = desc.parent;
// r.emplace<TeamComponent>(e, desc.team);
// BulletComponent bc;
// bc.bounce_bullet_on_wall_collision = desc.type == BulletType::BOUNCY;
// bc.type = desc.type;
// r.emplace<BulletComponent>(e, bc);
// set_position(r, e, desc.pos);