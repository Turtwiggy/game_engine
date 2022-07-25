// header
#include "system.hpp"

// components/systems
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

void
update_camera_system(entt::registry& registry)
{
  const auto& input = registry.ctx().at<SINGLETON_InputComponent>();
  const auto camera = get_main_camera(registry);
  auto& transform = registry.get<TransformComponent>(camera);

  const int CAM_SPEED = 2;

  if (get_key_held(input, SDL_SCANCODE_W))
    transform.position.y += 1 * CAM_SPEED;
  if (get_key_held(input, SDL_SCANCODE_S))
    transform.position.y -= 1 * CAM_SPEED;
  if (get_key_held(input, SDL_SCANCODE_A))
    transform.position.x += 1 * CAM_SPEED;
  if (get_key_held(input, SDL_SCANCODE_D))
    transform.position.x -= 1 * CAM_SPEED;
};

} // namespace game2d