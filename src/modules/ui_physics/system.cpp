// header
#include "system.hpp"

// components
#include "modules/physics/components.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_physics_system(entt::registry& registry)
{
  const auto& p = registry.ctx().at<SINGLETON_PhysicsComponent>();
  const auto& actors = registry.view<PhysicsActorComponent>();
  const auto& solids = registry.view<PhysicsSolidComponent>();

  ImGui::Begin("Physics", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("Solids %i", solids.size());
  ImGui::Text("Actors %i", actors.size());
  ImGui::Text("collision_enter %i", p.collision_enter.size());
  ImGui::Text("collision_stay %i", p.collision_stay.size());
  ImGui::Text("collision_exit %i", p.collision_exit.size());
  ImGui::End();
};