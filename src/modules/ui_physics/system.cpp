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

  const auto& objs = registry.view<PhysicsTransformComponent>();
  const auto& solids = registry.view<PhysicsSolidComponent>();
  const auto& actors = registry.view<PhysicsActorComponent>();
  const auto& stopped = registry.view<WasCollidedWithComponent>();

  ImGui::Begin("Physics", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("Objects %i", objs.size());
  ImGui::Text("Solids %i", solids.size());
  ImGui::Text("Actors %i", actors.size());
  ImGui::Text("WasCollidedWithComponent %i", stopped.size());
  ImGui::Text("collision_enter %i", p.collision_enter.size());
  ImGui::Text("collision_stay %i", p.collision_stay.size());
  ImGui::Text("collision_exit %i", p.collision_exit.size());
  ImGui::End();
};