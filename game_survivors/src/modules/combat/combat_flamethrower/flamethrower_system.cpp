#include "pch.hpp"

#include "flamethrower_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "flamethrower_components.hpp"
#include "modules/combat/combat_weapon_core/combat_weapon_core_components.hpp"

namespace game2d {

void
update_flamethrower_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // static float size_x = 100.0f;
  // static float size_y = 100.0f;
  // ImGui::Begin("DebugFlame");
  // imgui_draw_float("flame_size_x", size_x);
  // imgui_draw_float("flame_size_y", size_y);
  // ImGui::End();

  // rotate the flame
  {
    const auto view = r.view<const FlamethrowerFlameComponent, const HasParentComponent, TransformComponent>();
    for (const auto& [flame_e, flame_c, parent_c, t_c] : view.each()) {

      const auto par_e = parent_c.parent;

      // temp
      // t_c.scale.x = size_x;
      // t_c.scale.y = size_y;

      const auto& p_c = r.get<TransformComponent>(par_e);
      const auto& wep_t = p_c;

      const auto pos = glm::vec2{ wep_t.position.x, wep_t.position.y };
      const auto rot = wep_t.rotation_radians.z;
      const auto dir = engine::angle_radians_to_direction(rot);
      const auto off_pos = pos + dir * 0.5f * glm::vec2{ t_c.scale.x, t_c.scale.y };
      set_position(r, flame_e, off_pos);

      // todo: lerp the angle of the flame dont immediately update it
      t_c.rotation_radians.z = wep_t.rotation_radians.z;

      // set the rotation of the flame fixture.
      set_rotation(r, flame_e, wep_t.rotation_radians.z);
    }
  }

  // only show the flame when you're firing
  {
    const auto view = r.view<FlamethrowerFlameComponent, const HasParentComponent>();
    for (const auto& [e, flame_c, parent_c] : view.each()) {

      const auto par_e = parent_c.parent;
      const auto wep_e = par_e;

      const auto& wep_clip_size_c = r.get<WeaponClipSize>(wep_e);
      const auto& wep_reload_c = r.get<WeaponReloadRate>(wep_e);
      flame_c.active = wep_clip_size_c.bullets_cur >= 0;
      flame_c.active &= wep_reload_c.seconds_cur <= 0;
    }
  }
}

} // namespace game2d