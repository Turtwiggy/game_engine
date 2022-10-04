// header
#include "system.hpp"

// components/systems
#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
init_camera_system(entt::registry& registry)
{
  const auto& ri = registry.ctx().at<SINGLETON_RendererInfo>();

  // create a camera
  {
    auto c = create_gameplay(registry, EntityType::camera);
    registry.emplace<TransformComponent>(c);
  }

  // center all cameras
  const auto& cameras = registry.view<CameraComponent, TransformComponent>();
  for (auto [entity, camera, transform] : cameras.each()) {
    transform.position.x = -ri.viewport_size_render_at.x / 2;
    transform.position.y = -ri.viewport_size_render_at.y / 2;
  }
};

void
update_camera_system(entt::registry& registry, float dt)
{
  const auto& input = registry.ctx().at<InputComponent>();
  const auto& cameras = registry.view<CameraComponent, TransformComponent>();
  const float CAM_SPEED = 100.0f;

  for (auto [entity, camera, transform] : cameras.each()) {
    camera.view = calculate_view(transform, camera);

    if (get_key_held(input, SDL_SCANCODE_A)) {
      transform.position_dxdy.x -= cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y -= sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    } else if (get_key_held(input, SDL_SCANCODE_D)) {
      transform.position_dxdy.x += cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y += sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    }
    if (get_key_held(input, SDL_SCANCODE_S)) {
      transform.position_dxdy.x += -sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y += cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    } else if (get_key_held(input, SDL_SCANCODE_W)) {
      transform.position_dxdy.x -= -sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y -= cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    }

    int dx = static_cast<int>(transform.position_dxdy.x);
    if (glm::abs(dx) > 0) {
      transform.position.x += dx;
      transform.position_dxdy.x -= dx;
    }
    int dy = static_cast<int>(transform.position_dxdy.y);
    if (glm::abs(dy) > 0) {
      transform.position.y += dy;
      transform.position_dxdy.y -= dy;
    }

    // debug, recenter camera
    // if (get_key_down(input, SDL_SCANCODE_T)) {
    //   auto& ri = registry.ctx().at<SINGLETON_RendererInfo>();
    //   // center all cameras
    //   const auto& cameras = registry.view<CameraComponent, TransformComponent>();
    //   for (auto [entity, camera, transform] : cameras.each()) {
    //     transform.position.x = -ri.viewport_size_render_at.x / 2;
    //     transform.position.y = -ri.viewport_size_render_at.y / 2;
    //   }
    // }

    // if (get_mouse_mmb_held()) {
    //   // pan
    // if (get_mouse_lmb_held()) {
    //   // rotate
    // if (get_mouse_rmb_held()) {
    //   // zoom
  };
};

} // namespace game2d