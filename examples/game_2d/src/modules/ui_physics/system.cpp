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

  int solids = 0;
  int actors = 0;
  const auto& coll_view = registry.view<CollidableComponent>();
  coll_view.each([&solids, &actors](const auto& coll) {
    if (coll.type == PhysicsType::SOLID)
      solids += 1;
    else {
      actors += 1;
    }
  });

  ImGui::Begin("Physics", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  ImGui::Text("Collisions %i", p.frame_collisions.size());
  ImGui::Text("Persistent Collisions %i", p.persistent_collisions.size());
  ImGui::Text("Solids %i", solids);
  ImGui::Text("Actors %i", actors);
  ImGui::End();
};