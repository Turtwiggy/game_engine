// header
#include "system.hpp"

// components
#include "modules/physics/components.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_physics_system(entt::registry& registry, engine::Application& app)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  ImGui::Begin("Physics", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("Collisions %i", p.frame_collisions.size());
  ImGui::Text("Persistent Collisions %i", p.persistent_collisions.size());
  ImGui::End();
};