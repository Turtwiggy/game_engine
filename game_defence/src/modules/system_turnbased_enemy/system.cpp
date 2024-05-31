#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_turret/helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/grid/helpers.hpp"

#include "imgui.h"

namespace game2d {

std::vector<std::pair<entt::entity, int>>
get_players_in_range(entt::registry& r, const entt::entity& e)
{
  // get players within range.
  const auto entity = get_within_range<TurnBasedUnitComponent, TeamComponent>(r, e, 1000 * 1000);

  std::vector<std::pair<entt::entity, int>> players;
  for (const auto& [team_e, range] : entity) {
    const auto& team = r.get<TeamComponent>(team_e);
    if (team.team == AvailableTeams::player)
      players.push_back({ team_e, range });
  }

  // sort players by range
  std::sort(players.begin(), players.end(), [](const auto& l, const auto& r) { return l.second < r.second; });

  ImGui::Text("Players in range: %i", players.size());
  for (const auto& player : players)
    ImGui::Text("TargetPlayer: %i, range: %i", static_cast<uint32_t>(player.first), player.second);

  return players;
};

void
update_turnbased_enemy_system(entt::registry& r)
{
  const auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team != AvailableTeams::enemy)
    return; // only process system if enemy turn

  ImGui::Begin("Debug__TurnbasedEnemySystem");
  const bool one_at_a_time = false;
  bool all_enemies_fully_done = true;

  for (const auto& [e, e_c] : r.view<EnemyComponent>().each()) {
    ImGui::PushID(static_cast<uint32_t>(e));

    auto& turn_state = r.get_or_emplace<TurnState>(e);
    const bool has_moved = turn_state.has_moved;
    const bool has_shot = turn_state.has_shot;

    ImGui::Separator();
    ImGui::Text("has_moved %i", has_moved);
    ImGui::Text("has_shot: %i", has_shot);

    if (ImGui::Button("Do Move"))
      turn_state.do_move = true;
    if (ImGui::Button("Do Shoot"))
      turn_state.do_shoot = true;

    if (turn_state.do_move && !has_moved) {
      turn_state.has_moved = true;
      const auto players = get_players_in_range(r, e);
      if (players.size() > 0) {
        update_path_to_tile_next_to_player(r, e, players[0].first);
      }
    }
    if (turn_state.do_shoot && has_moved && !has_shot) {
      if (has_destination(r, e) && at_destination(r, e)) { // wait to arrive...
        turn_state.has_shot = true;                        // arrived! shoot if possible!
        const auto players = get_players_in_range(r, e);
        if (players.size() > 0) {
          r.emplace_or_replace<StaticTargetComponent>(e, get_position(r, players[0].first));
          r.emplace<WantsToShoot>(e);
        }
      }
    }

    // require all enemies to have done all actions
    all_enemies_fully_done &= has_moved;
    all_enemies_fully_done &= has_shot;

    ImGui::PopID();

    if (one_at_a_time && (!has_moved || !has_shot))
      break;
  }
  ImGui::End();

  // end the enemy turn
  if (all_enemies_fully_done)
    r.emplace<RequestToCompleteTurn>(r.create(), AvailableTeams::enemy);
}

} // namespace game2d