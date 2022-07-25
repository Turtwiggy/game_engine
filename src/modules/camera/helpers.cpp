// header
#include "helpers.hpp"

#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

entt::entity
get_main_camera(entt::registry& registry)
{
  const auto& cameras = registry.view<CameraComponent, TransformComponent>();
  const auto& main_camera = cameras.front();
  return main_camera;
};

} // namespace game2d