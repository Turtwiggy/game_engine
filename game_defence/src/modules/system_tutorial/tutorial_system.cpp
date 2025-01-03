#include "tutorial_system.hpp"

#include "tutorial_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_tutorial_system(entt::registry& r)
{
  ImGui::Begin("Tutorial");
  ImGui::Text("Todo...");
  ImGui::End();
}

} // namespace game2d