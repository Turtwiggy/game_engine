#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/system_turnbased/components.hpp"
#include "physics/helpers.hpp"

#include <algorithm>

// #include "imgui.h"

namespace game2d {

std::optional<Room>
get_a_room(entt::registry& r, const entt::entity& e)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto gen_e = get_first<DungeonGenerationResults>(r);
  if (gen_e == entt::null)
    return std::nullopt;
  const auto& gen = get_first_component<DungeonGenerationResults>(r);

  const auto e_pos = get_position(r, e);
  const auto e_gridpos = engine::grid::world_space_to_grid_space(e_pos, map.tilesize);
  AABB e_as_aabb = AABB();
  e_as_aabb.center = e_gridpos;
  e_as_aabb.size = { 1, 1 };

  for (const Room& room_e : gen.rooms)
    if (collide(room_e.aabb, e_as_aabb))
      return room_e;

  return std::nullopt;
};

std::vector<entt::entity>
get_players_in_room(entt::registry& r, const entt::entity& e)
{
  const auto gen_e = get_first<DungeonGenerationResults>(r);
  if (gen_e == entt::null)
    return {};
  const auto gen = get_first_component<DungeonGenerationResults>(r);

  std::vector<entt::entity> players;

  // either: be in the same room as a player
  const auto e_room = get_a_room(r, e);
  for (const auto& [player_e, tbc_c, team_c] : r.view<TurnBasedUnitComponent, TeamComponent>().each()) {
    if (team_c.team != AvailableTeams::player)
      continue;
    const auto player_room = get_a_room(r, player_e);
    if (player_room.has_value() && e_room.has_value() && e_room.value().tl == player_room.value().tl)
      players.push_back(player_e);
  }

  // get within range.
  const auto entity = get_within_range<TurnBasedUnitComponent, TeamComponent>(r, e, 100 * 100);
  for (const auto& [team_e, range] : entity) {
    const auto& team = r.get<TeamComponent>(team_e);
    if (team.team == AvailableTeams::player)
      players.push_back(team_e);
  }

  // sort players by range
  // std::sort(players.begin(), players.end(), [](const auto& l, const auto& r) { return l.second < r.second; });

  return players;
};

void
move_action(entt::registry& r, const entt::entity& e)
{
  const auto players = get_players_in_room(r, e);
  if (players.size() > 0) {
    const auto limit = r.get<MoveLimitComponent>(e).amount;
    update_path_to_tile_next_to_player(r, e, players[0], limit);
  }
  // choose a different random spot in the room...
  else {
    const auto update_path_to_rnd_idx_in_room = [&r, &e]() {
      const auto e_room = get_a_room(r, e);
      const auto& map = get_first_component<MapComponent>(r);

      // Convert Map to Grid (?)
      GridComponent grid;
      grid.size = map.tilesize;
      grid.width = map.xmax;
      grid.height = map.ymax;
      grid.grid = map.map;

      const auto src = get_position(r, e);
      const auto src_idx = convert_position_to_index(map, src);

      // should be any valid tiles, but for the moment, just choose a random one
      static engine::RandomState rnd;

      if (!e_room.has_value())
        return; // FIX: entity moved outside a room?

      const auto room = e_room.value();
      const int x = int(engine::rand_det_s(rnd.rng, room.tl.x, room.tl.x + room.aabb.size.x - 1));
      const int y = int(engine::rand_det_s(rnd.rng, room.tl.y, room.tl.y + room.aabb.size.y - 1));
      const int dst_idx = engine::grid::grid_position_to_index({ x, y }, map.xmax);
      auto dst = engine::grid::index_to_world_position(dst_idx, map.xmax, map.ymax, map.tilesize);
      dst += glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };

      auto path = generate_direct(r, grid, src_idx, dst_idx);

      // make sure path.size() < limit
      const auto limit = r.get<MoveLimitComponent>(e).amount;
      if (path.size() > limit) {
        // return +1, as usually the first path[0] is the element the entity is currently standing on
        const std::vector<glm::ivec2> path_limited(path.begin(), path.begin() + limit + 1);
        path = path_limited;
      }

      update_entity_path(r, e, path);
    };
    update_path_to_rnd_idx_in_room();
  }
}

void
shoot_action(entt::registry& r, const entt::entity& e)
{
  // shoot if possible!
  const auto players = get_players_in_room(r, e);
  if (players.size() > 0) {
    r.emplace_or_replace<StaticTargetComponent>(e, get_position(r, players[0]));
    r.emplace<WantsToShoot>(e);
  }
}

void
update_turnbased_enemy_system(entt::registry& r)
{
  const auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team != AvailableTeams::enemy)
    return; // only process system if enemy turn

  // ImGui::Begin("Debug__TurnbasedEnemySystem");
  const bool one_at_a_time = false;
  bool all_enemies_fully_done = true;

  for (const auto& [e, e_c] : r.view<EnemyComponent>().each()) {
    // ImGui::PushID(static_cast<uint32_t>(e));

    auto& action_state = r.get_or_emplace<ActionState>(e);

    // Enemies have X actions able to take.
    // They could either e.g. move and shoot,
    // or move and move if it made more sense.
    // For the moment, just make enemies move and shoot.

    int& actions_available = action_state.actions_available;
    int& actions_completed = action_state.actions_completed;

    // Force first action to be move
    if (actions_available > 0 && actions_completed == 0) {
      actions_available--;
      actions_completed = 1;
      move_action(r, e);
    }

    // Force second action to be shoot.
    if (actions_available > 0 && actions_completed == 1) {
      const bool at_dest = at_destination(r, e);
      if (at_dest) {
        actions_available--;
        actions_completed = 2;
        shoot_action(r, e);
      }
    }

    const bool waiting_to_shoot = r.try_get<WantsToShoot>(e) != nullptr;

    // require all enemies to have done all actions
    all_enemies_fully_done &= actions_available == 0;
    all_enemies_fully_done &= !waiting_to_shoot;

    // ImGui::PopID();
    if (one_at_a_time)
      break;
  }
  // ImGui::End();

  // end the enemy turn
  if (all_enemies_fully_done)
    create_empty<RequestToCompleteTurn>(r, RequestToCompleteTurn{ AvailableTeams::enemy });
}

} // namespace game2d