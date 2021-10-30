// your header
#include "systems/physics.hpp"

// components
#include "components/physics.hpp"
#include "components/rendering.hpp"

// helpers
#include "helpers/physics.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_physics_system(entt::registry& registry, engine::Application& app, float dt)
{
  std::vector<int> collisions;
  collisions.clear();

  ImGui::Begin("Physics Debug");
  {
    const auto& view = registry.view<const CollidableComponent>();
    view.each([](const auto& col) {
      //
      ImGui::Text("Physics Entity...");
    });
  }
  ImGui::End();
};