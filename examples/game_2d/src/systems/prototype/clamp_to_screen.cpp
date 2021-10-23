// your header
#include "systems/prototype/clamp_to_screen.hpp"

// components
#include "components/clamp_to_screen.hpp"
#include "components/physics.hpp"
#include "components/rendering.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_clamp_to_screen_system(entt::registry& registry, engine::Application& app, float dt)
{
  // ImGui::Begin("Parry", NULL, ImGuiWindowFlags_NoFocusOnAppearing);
  // ImGui::End();

  auto& view = registry.view<Velocity, const PositionInt, const ClampToScreen>();
  view.each([](auto& vel, const auto& pos, const auto& cts) {
    // TODO: need to be able to get screen size.
  });
}