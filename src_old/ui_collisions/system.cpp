#include "system.hpp"

#include "entt/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoBackground;

  ImGui::Begin("Collisions", NULL, flags);

  ImGui::Text("Collisions");

  ImGui::SameLine();
  ImGui::Text("Enter %i", physics.collision_enter.size());

  ImGui::SameLine();
  ImGui::Text("Stay %i", physics.collision_stay.size());

  ImGui::SameLine();
  ImGui::Text("Exit %i", physics.collision_exit.size());

  ImGui::End();
}

} // namespace game2d