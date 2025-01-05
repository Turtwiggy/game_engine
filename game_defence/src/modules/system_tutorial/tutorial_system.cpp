#include "tutorial_system.hpp"

#include "engine/entt/helpers.hpp"
#include "tutorial_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_tutorial_system(entt::registry& r)
{
  auto metrics_e = get_first<SINGLE_TutorialMetrics>(r);
  if (metrics_e == entt::null)
    return;
  auto& metrics_c = get_first_component<SINGLE_TutorialMetrics>(r);

  ImGui::Begin("Tutorial");

  ImGui::Text("Turns taken: %i", metrics_c.turns_taken);

  ImGui::End();
}

} // namespace game2d