#include "airlock_system.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_door/components.hpp"
#include "modules/map/components.hpp"
#include "modules/spaceship_designer/spaceship_designer_helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <ranges>

namespace game2d {
using namespace engine::grid;

// Either:
// If on the north or south side, show a button "enter" that will allow you in the airlock
// If it's not full and you click it, go in then airlock, and after X seconds, you're on the other side

// Or:
// If you're on the side of the airlock and nothings in it, open that door
// If you're on the side of the airlock but it's full, doors are closed

void
update_airlock_system(entt::registry& r)
{
  const auto map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map_c = r.get<MapComponent>(map_e);
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // generate a map from entity => grid position
  std::vector<std::pair<entt::entity, glm::ivec2>> m;
  const auto& mobs_view = r.view<const DefaultBody, TransformComponent>();
  for (const auto& [player_e, player_c, transform_c] : mobs_view.each()) {
    const auto pos = glm::vec2{ transform_c.position.x, transform_c.position.y };
    const auto gp = engine::grid::worldspace_to_grid_space(pos, map_c.tilesize);
    m.push_back({ player_e, gp });
  }

  // helper to search the entity => gridpos map values
  const auto contains_mobs = [&m](const glm::ivec2& gp) -> std::vector<entt::entity> {
    auto e_at_gp = m | std::views::filter([&gp](const auto& v) { return v.second == gp; }) |
                   std::views::transform([](const auto& v) { return v.first; }); // extract entt::entity
    return { std::ranges::begin(e_at_gp), std::ranges::end(e_at_gp) };
  };

  const auto& view = r.view<AirlockComponent>();
  for (const auto& [e, airlock_c] : view.each()) {
    // What gridpos is the airlock?
    const auto wp = get_position(r, e);
    const auto gp = worldspace_to_grid_space(wp, map_c.tilesize);
    const auto idx = grid_position_to_index(gp, map_c.xmax);

    const auto airlock_north = glm::ivec2{ gp.x, gp.y - 1 };
    const auto airlock_south = glm::ivec2{ gp.x, gp.y + 1 };

    const auto mobs_airlock = contains_mobs(gp);
    const auto mobs_north = contains_mobs(airlock_north);
    const auto mobs_south = contains_mobs(airlock_south);
    const auto mobs_in_airlock = mobs_airlock.size() > 0;
    const auto mobs_in_north = mobs_north.size() > 0;
    const auto mobs_in_south = mobs_south.size() > 0;
    const auto one_mob_in_north = mobs_north.size() == 1;
    const auto one_mob_in_south = mobs_north.size() == 1;
    const auto one_mob_in_airlock = mobs_airlock.size() == 1;

    const auto edge_north = edge_between_gps(r, gp, airlock_north);
    const auto edge_south = edge_between_gps(r, gp, airlock_south);
    const auto airlock_open_north = edge_north == entt::null;
    const auto airlock_open_south = edge_south == entt::null;

    if (edge_north != entt::null)
      airlock_c.north_edge_copy = r.get<Edge>(edge_north); // could work it out..
    if (edge_south != entt::null)
      airlock_c.south_edge_copy = r.get<Edge>(edge_south); // could work it out..

    // Something entered the airlock...
    //
    if (one_mob_in_airlock && airlock_c.mob_in_airlock == entt::null) {
      airlock_c.mob_in_airlock = mobs_airlock[0];
      airlock_c.entered_north = airlock_open_north;
      airlock_c.entered_south = airlock_open_south;
      fmt::println("mob entered airlock... (n){} (s){}", airlock_c.entered_north, airlock_c.entered_south);
    }

    // Something left the airlock...
    //
    if (!mobs_in_airlock && airlock_c.mob_in_airlock != entt::null) {
      airlock_c.mob_in_airlock = entt::null;
      fmt::println("mob exited airlock...");
    }

    // If the airlock is full...
    //
    if (airlock_c.mob_in_airlock != entt::null) {

      // how long to spend in airlock
      CooldownComponent cooldown;
      cooldown.time_max = 3.0f;
      cooldown.time = cooldown.time_max;
      const auto& cooldown_c = r.get_or_emplace<CooldownComponent>(e, cooldown);

      // ... and the airlock is ready to release...
      //
      const bool ready_to_open = cooldown_c.time <= 0.0f;
      const bool open = edge_north == entt::null || edge_south == entt::null;
      if (ready_to_open && !open) {
        fmt::println("airlock releasing...");

        if (airlock_c.entered_north) {
          fmt::println("removing south wall");
          dead.dead.emplace(edge_south);
        }
        if (airlock_c.entered_south) {
          fmt::println("removing north wall");
          dead.dead.emplace(edge_north);
        }
      }
      //
      // The airlock is not ready to release...
      // .. make sure all edges exist
      //
      if (!ready_to_open) {
        if (edge_north == entt::null) {
          auto edge_e = create_empty<Edge>(r, airlock_c.north_edge_copy);
          r.emplace<DoorComponent>(edge_e);
          instantiate_edges(r, map_c);
          fmt::println("closing north airlock wall...");
        }
        if (edge_south == entt::null) {
          auto edge_e = create_empty<Edge>(r, airlock_c.south_edge_copy);
          r.emplace<DoorComponent>(edge_e);
          instantiate_edges(r, map_c);
          fmt::println("closing south airlock wall...");
        }
      }
    }

    // If the airlock is empty...
    //
    if (airlock_c.mob_in_airlock == entt::null) {
      remove_if_exists<CooldownComponent>(r, e);

      //
      // if mobs above and an edge... delete edge
      //
      if (mobs_in_north && edge_north != entt::null) {
        airlock_c.north_edge_copy = r.get<Edge>(edge_north); // could work it out.. lazy
        dead.dead.emplace(edge_north);
        fmt::println("opening north airlock wall...");
      }
      if (mobs_in_south && edge_south != entt::null) {
        airlock_c.south_edge_copy = r.get<Edge>(edge_south); // could work it out.. lazy
        dead.dead.emplace(edge_south);
        fmt::println("opening south airlock wall...");
      }
      // If no mobs above and no edge... create edge
      //
      if (!mobs_in_north && edge_north == entt::null) {
        auto edge_e = create_empty<Edge>(r, airlock_c.north_edge_copy);
        r.emplace<DoorComponent>(edge_e);
        instantiate_edges(r, map_c);
        fmt::println("closing north airlock wall...");
      }
      if (!mobs_in_south && edge_south == entt::null) {
        auto edge_e = create_empty<Edge>(r, airlock_c.south_edge_copy);
        r.emplace<DoorComponent>(edge_e);
        instantiate_edges(r, map_c);
        fmt::println("closing south airlock wall...");
      }
    }
  }

  //
}

} // namespace game2d