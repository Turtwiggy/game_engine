#include "pch.hpp"

#include "actor_player_helpers.hpp"

namespace game2d {

bool
has_action(const std::vector<ActionStateEnum>& action, const ActionStateEnum act)
{
  auto it = std::find(action.begin(), action.end(), act);
  return it != action.end();
};

} // namespace game2d