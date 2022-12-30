#include "animated_cursor_click.hpp"

// components
#include "components.hpp"
#include "events/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

// helpers
#include "events/helpers/mouse.hpp"

namespace game2d {

void
update_animated_cursor_click_system(entt::registry& registry)
{
  // const auto& input = registry.ctx().at<SINGLETON_InputComponent>();
  // const auto& view = registry.view<SpriteComponent, SpriteAnimationComponent, TransformComponent>();
  // view.each([&input](auto entity, const auto& sprite, auto& animation, auto& transform) {
  //   if (get_mouse_lmb_press()) {
  //     animation.playing = true;
  //     animation.frame = 1; // frame 0 is empty
  //     transform.position = { input.mouse_position_in_worldspace.x, input.mouse_position_in_worldspace.y, 0.0f };
  //   }
  // });
}

} // namespace game2d