#include "system.hpp"

#include "entt/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLE_Physics>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  // flags |= ImGuiWindowFlags_NoDecoration;

  ImGui::Begin("Collisions", NULL, flags);

  ImGui::SameLine();
  ImGui::Text("BodyCount(): %i", physics.world->GetBodyCount());

  ImGui::SameLine();
  ImGui::Text("ContactCount(): %i", physics.world->GetContactCount());

  ImGui::End();
}

} // namespace game2d