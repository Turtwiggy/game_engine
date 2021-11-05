
// your header
#include "systems/ai_head_to_random_point.hpp"

// components
#include "components/ai_head_to_random_point.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ai_head_to_random_point_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& view = registry.view<AIHeadToRandomPoint, VelocityComponent>();
  view.each([](auto& ai, auto& vel) {
    //
    // vel.x = 10.0f;
    // vel.y = 10.0f;
  });
};
