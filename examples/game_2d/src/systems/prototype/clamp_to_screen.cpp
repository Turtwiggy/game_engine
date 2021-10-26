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
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();

  glm::ivec2 boundary = ri.viewport_size;

  const auto& view = registry.view<Velocity, const PositionInt, const Size, const ClampToScreen>();
  view.each([&boundary](auto& vel, const auto& pos, const auto& size, const auto& cts) {
    if (pos.x - (size.w / 2.0f) <= 0.0f)
      vel.x = abs(vel.x);
    if (pos.x + (size.w / 2.0f) >= boundary.x)
      vel.x = -abs(vel.x);
    if (pos.y - (size.h / 2.0f) <= 0.0f)
      vel.y = abs(vel.y);
    if (pos.y + (size.h / 2.0f) >= boundary.y)
      vel.y = -abs(vel.y);
  });
}