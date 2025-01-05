#include "tutorial_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "tutorial_components.hpp"

namespace game2d {

void
handle_end_turn_tutorial(entt::registry& r, const EndTurnEvent& evt)
{
  const auto metrics_e = get_first<SINGLE_TutorialMetrics>(r);
  if (metrics_e == entt::null)
    return;
  auto& metrics_c = get_first_component<SINGLE_TutorialMetrics>(r);
  metrics_c.turns_taken++;
};

} // namespace game2d