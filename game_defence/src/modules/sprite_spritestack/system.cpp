#include "system.hpp"

#include "components.hpp"
#include "imgui/helpers.hpp"
#include "renderer/transform.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

namespace game2d {

void
update_sprite_spritestack_system(entt::registry& r, const float dt)
{
  static float scale_up_by = 2.0f;
  static int sprite_height = 1;

  ImGui::Begin("DebugSpriteStack");
  imgui_draw_float("scale", scale_up_by);
  imgui_draw_int("sprite_height", sprite_height);

  const auto& gun_view = r.view<TransformComponent, SpritestackComponent, RotateOnSpotComponent>();
  for (const auto& [e, t, ssc, r] : gun_view.each()) {
    t.position.y = 0 + scale_up_by * sprite_height * ssc.spritestack_position;

    //
    // t.scale.x = scale_up_by * 15;
    // t.scale.y = scale_up_by * 34;
    // t.scale.x = scale_up_by * 20;
    // t.scale.y = scale_up_by * 21;

    t.scale.x = scale_up_by * 5;
    t.scale.y = scale_up_by * 16;

    t.rotation_radians.z += dt;
  }

  ImGui::End();
}

} // namespace game2d