#include "helpers.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "audio/components.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_breached_room/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_inventory/helpers.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"

#include <unordered_set>

namespace game2d {

glm::ivec2
calculate_oob_edge_gp(const MapComponent& map, glm::ivec2 xy)
{
  glm::ivec2 edge_oob{ 0, 0 };

  if (xy.x == 0)
    edge_oob = { -1, xy.y };

  if (xy.y == 0)
    edge_oob = { xy.x, -1 };

  if (xy.x == map.xmax)
    edge_oob = { xy.x + 1, xy.y };

  if (xy.y == map.ymax)
    edge_oob = { xy.x, xy.y + 1 };

  return edge_oob;
};

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

    std::unordered_set<Edge, EdgeHash> edges_blown_up;

    // Have to use gridpos not index, because gridpos {-1, 12} is valid for a bomb to be placed
    // but if that was convertd to an index it would wrap round the value and be too large.
    const auto neighbours = engine::grid::get_neighbour_gridpos(gp, map.xmax, map.ymax);
    for (const auto& [dir, n_gp] : neighbours) {

      // Note: for a_idx in the edge representation,
      // a_idx is in bounds, and b_idx is out of bounds,
      // representing that an edge is on the border.
      // This means that one gp is in bounds, and one is out of bounds,
      // or both gp are in bounds.

      const auto gp_bomb = gp; // where the bomb is placed
      const bool gp_bomb_oob_x = gp_bomb.x < 0 || gp_bomb.x >= map.xmax;
      const bool gp_bomb_oob_y = gp_bomb.y < 0 || gp_bomb.y >= map.ymax;
      const bool bomb_out_of_bounds = gp_bomb_oob_x || gp_bomb_oob_y;

      const auto gp_neighbour = n_gp; // a neighbouring grid position
      const bool gp_neighbour_oob_x = gp_neighbour.x < 0 || gp_neighbour.x >= map.xmax;
      const bool gp_neighbour_oob_y = gp_neighbour.y < 0 || gp_neighbour.y >= map.ymax;
      const bool neighbour_out_of_bounds = gp_neighbour_oob_x || gp_neighbour_oob_y;

      const auto pred = [&](const Edge& other) -> bool {
        //
        // No edge (should) be valid with these conditions
        if (bomb_out_of_bounds && neighbour_out_of_bounds)
          return false;

        // one of the edges is out of bound, and we're on an edge that has an out of bound edge.
        if ((bomb_out_of_bounds || neighbour_out_of_bounds) && (other.b_idx == -1)) {

          // the edge's in-bound edge.
          const auto xy = engine::grid::index_to_grid_position(other.a_idx, map.xmax, map.ymax);

          // the edge's out-of-bound edge.
          const auto edge_oob = calculate_oob_edge_gp(map, xy);

          glm::ivec2 inb_gp{ 0, 0 };
          glm::ivec2 oob_gb{ 0, 0 };
          if (bomb_out_of_bounds) {
            oob_gb = gp_bomb;
            inb_gp = gp_neighbour;
          }
          if (neighbour_out_of_bounds) {
            oob_gb = gp_neighbour;
            inb_gp = gp_bomb;
          }

          // Pull the lever, kronk!
          return xy == inb_gp && edge_oob == oob_gb;
        }

        // The only case we have left:
        // both bomb and neighbour grid positions are in bounds
        Edge ed;
        ed.a_idx = engine::grid::grid_position_to_index(gp, map.xmax);
        ed.b_idx = engine::grid::grid_position_to_index(n_gp, map.xmax);

        // do a swap because the map.edges() expects a < b, except for the case that b is -1
        if (ed.a_idx > ed.b_idx)
          std::swap(ed.a_idx, ed.b_idx);

        return ed == other;
      };

      const auto it = std::find_if(map.edges.begin(), map.edges.end(), pred);

      if (it != map.edges.end()) {

        const entt::entity instance = it->instance;
        if (instance == entt::null)
          continue;

        edges_blown_up.emplace(*it);
        fmt::println("removing edge..");

        // destroy body immediately
        auto& physics_b = r.get<PhysicsBodyComponent>(instance).body;
        physics.world->DestroyBody(physics_b);
        r.remove<PhysicsBodyComponent>(instance);

        // destroy transform
        dead.dead.emplace(instance);

        // destroy edge
        map.edges.erase(it);
      }
    }

    fmt::println("resulting edges blown up: {}", edges_blown_up.size());
    //
    // create a particle spawner...
    //
    for (const Edge& edge : edges_blown_up) {

      const auto pos_a = engine::grid::index_to_grid_position(edge.a_idx, map.xmax, map.ymax);

      auto pos_b = glm::vec2{ 0.0f, 0.0f };
      if (edge.b_idx != -1)
        pos_b = engine::grid::index_to_grid_position(edge.b_idx, map.xmax, map.ymax);
      if (edge.b_idx == -1)
        pos_b = calculate_oob_edge_gp(map, pos_a);

      const auto pos_a_worldspace = engine::grid::grid_space_to_world_space_center(pos_a, map.tilesize);
      const auto pos_b_worldspace = engine::grid::grid_space_to_world_space_center(pos_b, map.tilesize);

      const auto rooms_a = inside_room(r, pos_a);
      const auto rooms_b = inside_room(r, pos_b);
      const auto in_room = rooms_a.size() > 0 || rooms_b.size() > 0;
      const auto in_tunnel = inside_tunnels(dungeon.tunnels, pos_a).size() > 0;

      // add a breachedroom component...
      for (const auto room_e : rooms_a)
        r.emplace_or_replace<BreachedRoom>(room_e);
      for (const auto room_e : rooms_b)
        r.emplace_or_replace<BreachedRoom>(room_e);

      // Point the particles outside the room
      glm::vec2 dir{ 0.0f, 0.0f };
      if (in_room || in_tunnel)
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

std::pair<bool, entt::entity>
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
        return { true, slots[0] };
    }
  }

  return { false, entt::null };
};

} // namespace game2d