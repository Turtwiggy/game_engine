#include "pch.hpp"

#include "engine/renderer/transform.hpp"
#include "lighthouse_components.hpp"
#include "lighthouse_system.hpp"

namespace game2d {

void
update_actor_lighthouse_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  static float rotate_speed = 0.1f;
  // imgui_draw_float("rotate_speed", rotate_speed);

  const auto view = r.view<LighthouseComponent, TransformComponent>();
  for (const auto& [e, lighthouse_c, t_c] : view.each()) {
    t_c.rotation_radians.z += dt * rotate_speed;
  }
}

} // namespace game2d