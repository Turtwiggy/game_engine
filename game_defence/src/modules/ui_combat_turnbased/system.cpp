#include "system.hpp"

#include "components.hpp"

#include "imgui.h"
#include "modules/ux_hoverable/components.hpp"

namespace game2d {

// check if in range
// if in range,
// give all entities in the fight an initiative.
// some entities may join and leave combat
// update_combat_engagement_system(r);
// update_combat_turnbased_system(r);

void
update_ui_combat_turnbased_system(entt::registry& r)
{
  ImGui::Begin("TurnbasedCombat");

  ImGui::Text("Hello, Combat!");

  const auto& selected_view = r.view<SelectedComponent>();
  for (const auto& [e, selected_c] : selected_view.each()) {
    ImGui::Text("Something selected");
  }

  ImGui::End();
}

} // namespace game2d