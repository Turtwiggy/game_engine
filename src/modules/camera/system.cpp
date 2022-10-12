// header
#include "system.hpp"

// components/systems
#include "game/components/actors.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/renderer/components.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
init_camera_system(GameEditor& editor, Game& game, glm::ivec2 camera_target)
{
  const auto& ri = editor.renderer;
  auto& registry = game.state;

  // create a camera
  {
    auto c = create_gameplay(editor, game, EntityType::camera);
    registry.emplace<TransformComponent>(c);
  }

  // center all cameras
  const auto& cameras = registry.view<CameraComponent, TransformComponent>();
  for (auto [entity, camera, transform] : cameras.each()) {
    transform.position.x = camera_target.x;
    transform.position.y = camera_target.y;
  }
};

void
update_camera_system(const GameEditor& editor, Game& game, float dt)
{
  const auto& input = game.input;
  auto& registry = game.state;
  const auto& cameras = registry.view<CameraComponent, TransformComponent>();

  float CAM_SPEED = 100.0f;
  if (get_key_held(input, SDL_SCANCODE_LSHIFT))
    CAM_SPEED *= 2.0f;

  for (auto [entity, camera, transform] : cameras.each()) {
    camera.view = calculate_view(transform, camera);

    if (get_key_held(input, SDL_SCANCODE_LEFT)) {
      transform.position_dxdy.x -= cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y -= sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    } else if (get_key_held(input, SDL_SCANCODE_RIGHT)) {
      transform.position_dxdy.x += cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y += sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    }
    if (get_key_held(input, SDL_SCANCODE_DOWN)) {
      transform.position_dxdy.x += -sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
      transform.position_dxdy.y += cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
    } else if (get_key_held(input, SDL_SCANCODE_UP)) {
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