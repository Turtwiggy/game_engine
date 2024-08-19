#include "system.hpp"

#include "components.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/system_particles/components.hpp"
#include "physics/components.hpp"

#include <fmt/core.h>

namespace game2d {

void
update_breach_charge_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  glm::ivec2 mouse_pos_on_grid = engine::grid::worldspace_to_clamped_world_space(mouse_pos, map.tilesize);
  mouse_pos_on_grid += offset;

  if (get_mouse_lmb_press()) {
    // spawn a breach charge
    const auto charge_e = create_gameplay(r, EntityType::actor_breach_charge, mouse_pos_on_grid);

    r.emplace<EntityTimedLifecycle>(charge_e, 3 * 1000);

    OnDeathCallback callback;
    callback.callback = [](entt::registry& r, const entt::entity& e) {
      // create a boom effect
      const glm::vec2 pos = get_position(r, e);
      fmt::println("go boom at {}, {}", pos.x, pos.y);
      create_empty<RequestToSpawnParticles>(r, RequestToSpawnParticles{ pos });

      // open up any edges surrounding the boom
      // Did I accidentally create bomberman?
      auto& physics = get_first_component<SINGLE_Physics>(r);
      auto& map = get_first_component<MapComponent>(r);
      auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
      const auto gp = engine::grid::worldspace_to_grid_space(pos, map.tilesize);
      const auto neighbours = engine::grid::get_neighbour_indicies(gp.x, gp.y, map.xmax, map.ymax);
      for (const auto& [dir, n_idx] : neighbours) {

        Edge ed;
        ed.a_idx = engine::grid::grid_position_to_index(gp, map.xmax);
        ed.b_idx = n_idx;
        if (ed.a_idx > ed.b_idx)
          std::swap(ed.a_idx, ed.b_idx);

        const auto pred = [&ed](const Edge& other) { return ed == other; };
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
    r.emplace<OnDeathCallback>(charge_e, callback);
  }

  //
}

} // namespace game2d