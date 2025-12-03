#include "pch.hpp"

#include "spritestack_system.hpp"

#include "spritestack_components.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_sprite_spritestack_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // note: values of 1 seem to distort the spritestack, but add depth
  // because it SHOULD be in top-down perspective
  // values of 0 are just top down perspective,
  // so a value of 0-1 is recommended between these two.
  float scale_up_by = 0.6f;
  int sprite_height = 1;
  int parallax_offset_amount = 0;

#if defined(_DEBUG)
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  const auto& ui_state = gesert_menubar_state(menu_c, "(Debug) Spritestack");
  if (ui_state.enabled) {
    ImGui::Begin("Debug Spritestack");
    imgui_draw_float("scale", scale_up_by);
    imgui_draw_int("sprite_height", sprite_height);
    imgui_draw_int("parallax_offset_amount", parallax_offset_amount);
    ImGui::End();
  }
#endif

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto camera_pos = get_position(r, camera_e);
  const auto& anims = SINGLE_Animations::instance;

  const auto& view = r.view<TransformComponent, SpritestackComponent, SpriteComponent>();
  for (const auto& [e, t, ssc, sprite] : view.each()) {
    const auto idx = ssc.spritestack_index;

    // HACK: parent is ded. How to cleanup spritestacks?
    if (!r.valid(ssc.root) || ssc.root == entt::null) {
      dead.dead.push_back(e);
      continue;
    }

    const auto [a, b] = find_animation(anims, ssc.tag);
    const auto& spritesheet = anims.animations[a].first;
    const int sprite_scale_x = spritesheet.px;
    const int sprite_scale_y = spritesheet.py;

    // hack: make the trimaran half the size
    // if (ssc.tag.find("trimaran") != std::string::npos) {
    //   sprite_scale_x *= 0.5;
    //   sprite_scale_y *= 0.5;
    //   scale_up_by = 0.6f * 0.5f;
    // }

    // Set position for each child sprite
    // if (ssc.spritestack_index != 0)
    {
      const auto sprite_parent = ssc.root;
      const auto& sprite_parent_transform = r.get<TransformComponent>(sprite_parent);

      // set position to parents position
      t.position = sprite_parent_transform.position;

      // set rotation to parents rotation
      t.rotation_radians.z = sprite_parent_transform.rotation_radians.z;
    }

    t.position.y += scale_up_by * (sprite_height * idx);
    t.scale = { sprite_scale_x, sprite_scale_y, 1.0f };

    // adjust colour
    // float percent = ((ssc.spritestack_total - 1) - ssc.spritestack_index);
    // sprite.colour.a = (ssc.spritestack_index + 1.0f) / float(ssc.spritestack_total);

    // parallax effect
    // const glm::vec2 dir_raw = camera_pos - glm::vec2{ t.position.x, t.position.y };
    // const glm::vec2 dir_nrm = engine::normalize_safe(dir_raw);
    // const glm::vec2 offset = { -dir_nrm.x * idx * parallax_offset_amount, -dir_nrm.y * idx * parallax_offset_amount };
    // t.position.x += offset.x;
    // t.position.y += offset.y;
  }

  // if (ui_state.enabled)
  //   ImGui::End();
}

} // namespace game2d