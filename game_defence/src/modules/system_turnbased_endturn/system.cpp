#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/system_change_gun_colour/helpers.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_colours/helpers.hpp"

#include <fmt/core.h>

namespace game2d {

void
teleport_to_destination(entt::registry& r, const entt::entity e)
{
  if (has_destination(r, e) && !at_destination(r, e)) {
    const auto& path = r.get<GeneratedPathComponent>(e);
    set_position(r, e, path.dst_pos);
  }
}

void
update_turnbased_endturn_system(entt::registry& r)
{
  // Only process system if request
  const auto& req_view = r.view<RequestToCompleteTurn>();
  if (req_view.size() == 0)
    return;
  const auto request = get_first_component<RequestToCompleteTurn>(r);
  r.destroy(req_view.begin(), req_view.end());
  // fmt::println("End turn request...");

  // If anything is left moving, teleport it
  // for (const auto& [e, ts_c] : r.view<ActionState>().each())
  //   teleport_to_destination(r, e);

  // At end of turn, Destroy everything's turnstate
  const auto reqs = r.view<ActionState>();
  r.remove<ActionState>(reqs.begin(), reqs.end());

  auto& turn_state = get_first_component<SINGLE_CombatState>(r);
  const auto& current_team_turn = turn_state.team;
  const auto& team_request_to_end_turn = request.team;

  if (current_team_turn != team_request_to_end_turn)
    return; // request to end turn that doesnt exist
  // else, valid request...

  const bool is_player_turn = current_team_turn == AvailableTeams::player;
  const bool is_enemy_turn = current_team_turn == AvailableTeams::enemy;

  if (is_player_turn)
    turn_state.team = AvailableTeams::enemy;
  else if (is_enemy_turn)
    turn_state.team = AvailableTeams::player;

  //
};

} // namespace game2d