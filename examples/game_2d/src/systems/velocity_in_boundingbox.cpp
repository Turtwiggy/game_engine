// your header
#include "systems/velocity_in_boundingbox.hpp"

// components
#include "components/velocity_in_boundingbox.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other engine headers
#include "engine/grid.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_velocity_in_boundingbox_system(entt::registry& registry, engine::Application& app, float dt)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();

  glm::ivec2 boundary = ri.viewport_size_render_at;

  const auto& view = registry.view<VelocityComponent,
                                   const PositionIntComponent,
                                   const PhysicsSizeComponent,
                                   const VelocityInBoundingboxComponent>();

  view.each([&boundary](auto& vel, const auto& pos, const auto& size, const auto& vib) {
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