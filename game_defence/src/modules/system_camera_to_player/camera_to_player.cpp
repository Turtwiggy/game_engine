#include "camera_to_player.hpp"

#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"

namespace game2d {

void
update_swap_camera_to_player_system(entt::registry& r)
{
  const auto& view = r.view<PlayerComponent>();
  for (const auto& [e, player_c] : view.each()) {

    const auto* cam_focus = r.try_get<CameraFollow>(e);
    if (cam_focus) {
      if (!r.any_of<InputComponent>(e)) {
        r.emplace<InputComponent>(e);
        r.emplace<KeyboardComponent>(e);
      }
    }

    if (!cam_focus) {
      if (r.any_of<InputComponent, KeyboardComponent>(e)) {
        r.remove<InputComponent>(e);
        r.remove<KeyboardComponent>(e);
      }
    }

    //
  }
}

} // namespace game2d