#include "helpers.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"
#include "physics/components.hpp"

#include <unordered_set>

namespace game2d {

void
create_o2_particle(entt::registry& r, entt::entity dead_e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto& edge = r.get<Edge>(dead_e);
  const auto pos_a_worldspace = engine::grid::grid_space_to_world_space_center(edge.gp_a, map.tilesize);
  const auto pos_b_worldspace = engine::grid::grid_space_to_world_space_center(edge.gp_b, map.tilesize);

  const auto rooms_a = inside_room(r, edge.gp_a);
  const auto rooms_b = inside_room(r, edge.gp_b);
  const auto in_room = rooms_a.size() > 0 || rooms_b.size() > 0;

  // add a breachedroom component...
  // for (const auto room_e : rooms_a)
  //   r.emplace_or_replace<BreachedRoom>(room_e);
  // for (const auto room_e : rooms_b)
  //   r.emplace_or_replace<BreachedRoom>(room_e);

  // Point the particles outside the room
  glm::vec2 dir{ 0.0f, 0.0f };
  if (in_room)
    dir = engine::normalize_safe(pos_b_worldspace - pos_a_worldspace);
  else
    dir = -engine::normalize_safe(pos_b_worldspace - pos_a_worldspace);

  const auto halfway_pos = (pos_a_worldspace + pos_b_worldspace) * 0.5f;

  // a particle_parent_e is what the particle emitter follows
  const auto particle_parent_e = create_transform(r);
  r.get<TagComponent>(particle_parent_e).tag = "particle_emitter_parent";
  set_position(r, particle_parent_e, halfway_pos);
  set_size(r, particle_parent_e, { 0, 0 }); // no size for particle emitter

  // add_particles()
  DataParticleEmitter desc;
  desc.parent = particle_parent_e;
  desc.velocity = dir * 20.0f;
  desc.start_size = 10;
  desc.end_size = 2;
  desc.colour = engine::SRGBColour{ 0.9f, 0.9f, 0.9f, 0.9f };
  auto particle_e = Factory_DataParticleEmitter::create(r, desc);

  fmt::println("spawning particle emitter due to blown up edge at {} {}", halfway_pos.x, halfway_pos.y);
}

void
add_bomb_callback(entt::registry& r, const entt::entity e)
{
  OnDeathCallback callback;
  callback.callback = [](entt::registry& r, const entt::entity e) {
    const auto& dungeon = get_first_component<DungeonGenerationResults>(r);

    // create a boom effect
    const glm::vec2 pos = get_position(r, e);
    create_empty<RequestToSpawnParticles>(r, RequestToSpawnParticles{ pos });

    // open up any edges surrounding the boom
    // Did I accidentally create bomberman?
    auto& physics = get_first_component<SINGLE_Physics>(r);
    auto& map = get_first_component<MapComponent>(r);
    auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map.tilesize);
    // fmt::println("go boom at {}, {}. gp: {}, {}", pos.x, pos.y, gp.x, gp.y);

    // Have to use gridpos not index, because gridpos {-1, 12} is valid for a bomb to be placed
    // but if that was convertd to an index it would wrap round the value and be too large.
    const auto neighbours = engine::grid::get_neighbour_gridpos(gp, map.xmax, map.ymax);
    for (const auto& [dir, n_gp] : neighbours) {

      const entt::entity edge_e = edge_between_gps(r, gp, n_gp);
      if (edge_e != entt::null) {
        const Edge& edge = r.get<Edge>(edge_e);
        fmt::println("removing edge..");

        // destroy body immediately
        auto& physics_b = r.get<PhysicsBodyComponent>(edge_e).body;
        physics.world->DestroyBody(physics_b);
        r.remove<PhysicsBodyComponent>(edge_e);

        // destroy edge
        dead.dead.emplace(edge_e);
      }
    }

    // create a particle spawner...
    for (const auto& dead_e : dead.dead)
      if (auto* edge_c = r.try_get<Edge>(dead_e))
        create_o2_particle(r, dead_e);

    // request some audio
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "BOMB_BLOWUP_01" });
  };
  r.emplace<OnDeathCallback>(e, callback);
};

bool
debug_spawn_bombs(entt::registry& r, const glm::ivec2& mouse_pos_on_grid)
{
  // debug spawn bombs immediately
  bool imediately_place_bomb = false;

#if defined(_DEBUG)
  imediately_place_bomb = false;
#endif

  if (imediately_place_bomb && get_mouse_lmb_press()) {
    fmt::println("immediately placing bomb...");
    DataBreachCharge desc;
    desc.pos = glm::vec2(mouse_pos_on_grid);
    const auto charge_e = Factory_DataBreachCharge::create(r, desc);
  }

  return imediately_place_bomb;
};

entt::entity
bomb_equipped_in_inventory(entt::registry& r)
{
  const auto& view = r.view<const PlayerComponent, DefaultBody>();

  for (const auto& [e, player_c, equipment_c] : view.each()) {
    bool able_to_use_breach_charge = false;
    // std::vector<entt::entity> equipment = equipment_c.body;

    const auto slots = get_slots(r, e, InventorySlotType::gun);
    if (slots.size() > 0) {
      const auto equipment_slot_e = slots[0];
      const auto equipment_slot = r.get<InventorySlotComponent>(equipment_slot_e);
      const auto has_item = equipment_slot.item_e != entt::null;
      if (!has_item)
        continue;

      // Is the item a breach charge?
      if (r.get<ItemTypeComponent>(equipment_slot.item_e).type == ItemType::bomb)
        return slots[0];
    }
  }

  return entt::null;
};

} // namespace game2d