#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"

namespace game2d {

void
update_turnbased_endturn_system(entt::registry& r)
{
  // Only process system if request
  const auto& req_view = r.view<RequestToCompleteTurn>();
  if (req_view.size() == 0)
    return;
  const auto request = get_first_component<RequestToCompleteTurn>(r);
  r.destroy(req_view.begin(), req_view.end());

  // At end of turn, Destroy everything's turnstate
  const auto& view = r.view<TurnState>();
  r.remove<TurnState>(view.begin(), view.end());

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