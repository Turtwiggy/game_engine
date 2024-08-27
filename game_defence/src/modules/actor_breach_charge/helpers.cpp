#include "helpers.hpp"

#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_particles/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
add_bomb_callback(entt::registry& r, const entt::entity e)
{
  const auto& map = get_first_component<MapComponent>(r);

  OnDeathCallback callback;
  callback.callback = [](entt::registry& r, const entt::entity e) {
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

          // get the other edge grid pos
          glm::ivec2 edge_oob{ 0, 0 };
          if (xy.x == 0)
            edge_oob = { -1, xy.y };
          if (xy.y == 0)
            edge_oob = { xy.x, -1 };
          if (xy.x == map.xmax)
            edge_oob = { xy.x + 1, xy.y };
          if (xy.y == map.ymax)
            edge_oob = { xy.x, xy.y + 1 };

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

        // destroy body immediately
        auto& physics_b = r.get<PhysicsBodyComponent>(instance).body;
        physics.world->DestroyBody(physics_b);
        r.remove<PhysicsBodyComponent>(instance);

        // destroy transform
        dead.dead.emplace(instance);

        // destroy edge
        map.edges.erase(it);
        fmt::println("removed edge..");
      }

      // TODO: everything in that room (or tunnel) gets sucked out
    }
  };
  r.emplace<OnDeathCallback>(e, callback);
};

} // namespace game2d