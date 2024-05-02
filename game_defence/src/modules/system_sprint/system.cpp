#include "system.hpp"

#include "components.hpp"
#include "lifecycle/components.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {

void
update_sprint_system(entt::registry& r, const float dt)
{
  static bool debug_sprint = true;
  ImGui::Begin("Debug__Sprint", &debug_sprint);

  // Do the sprint
  const auto& sprintable = r.view<SprintComponent, WantsToSprint>(entt::exclude<WaitForInitComponent>);
  for (const auto& [sprint_e, sprint_c, sprint_request] : sprintable.each()) {
    const auto* stop_sprint = r.try_get<WantsToReleaseSprint>(sprint_e);
    if (stop_sprint) {
      r.remove<WantsToSprint>(sprint_e);
      r.remove<WantsToReleaseSprint>(sprint_e);
      continue;
    }

    // Reflect in state
    sprint_c.is_sprinting = sprint_c.sprint_left_seconds > 0.0f;
    ImGui::Text("EntitySprinting: %i. %f", sprint_c.is_sprinting, sprint_c.sprint_left_seconds);

    // Limit sprint
    if (sprint_c.sprint_left_seconds > 0.0f) {
      sprint_c.sprint_left_seconds -= dt;
      sprint_c.sprint_left_seconds = glm::max(sprint_c.sprint_left_seconds, 0.0f);
    }
  }

  // Recharge Sprint
  //
  const auto& not_sprinting = r.view<SprintComponent>(entt::exclude<WantsToSprint, WaitForInitComponent>);
  for (const auto& [sprint_e, sprint_c] : not_sprinting.each()) {
    sprint_c.is_sprinting = false;
    sprint_c.sprint_left_seconds += dt;
    sprint_c.sprint_left_seconds = glm::min(sprint_c.sprint_left_seconds, sprint_c.sprint_max_seconds);
    ImGui::Text("EntityRecharging: %f", sprint_c.sprint_left_seconds);
  }

  ImGui::End();
}

} // namespace game2d