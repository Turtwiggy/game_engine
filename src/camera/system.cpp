// header
#include "system.hpp"

// components/systems
#include "maths/grid.hpp"
#include "components/actors.hpp"
#include "modules/player/components.hpp"
#include "camera/components.hpp"
#include "camera/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "renderer/components.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

namespace game2d {

// VARS
const int GRID_SIZE = 16;
static float damping = 0.5f;
static glm::vec3 target_position;

void
update_camera_system(const GameEditor& editor, Game& game, float dt)
{
  const auto& input = game.input;
  auto& r = game.state;
  const auto& cameras = r.view<CameraComponent, TransformComponent>();

  float CAM_SPEED = 100.0f;
  if (get_key_held(input, SDL_SCANCODE_LSHIFT))
    CAM_SPEED *= 2.0f;

  // TRIAL: camera follow player?
  const auto& player_view = r.view<PlayerComponent>();
  if (player_view.size() == 0)
    return; // no player to home in on
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  const auto player_grid =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, GRID_SIZE);
  glm::ivec2 player_position = engine::grid::grid_space_to_world_space(player_grid, 16);

  const auto& ri = editor.renderer;

  target_position = { (-ri.viewport_size_render_at.x / 2.0f) + player_position.x,
                      (-ri.viewport_size_render_at.y / 2.0f) + player_position.y,
                      0.0f };

  for (auto [entity, camera, transform] : cameras.each()) {
    camera.view = calculate_view(transform, camera);

    glm::vec3 pos_as_vec3 = glm::vec3(transform.position.x, transform.position.y, transform.position.z);
    glm::vec3 lerp_pos = glm::lerp(pos_as_vec3, target_position, glm::clamp(dt * damping, 0.0f, 1.0f));
    transform.position = {
      static_cast<int>(lerp_pos.x),
      static_cast<int>(lerp_pos.y),
      static_cast<int>(lerp_pos.z),
    };
  }

  // if (get_key_held(input, SDL_SCANCODE_LEFT)) {
  //   transform.position_dxdy.x -= cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  //   transform.position_dxdy.y -= sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  // } else if (get_key_held(input, SDL_SCANCODE_RIGHT)) {
  //   transform.position_dxdy.x += cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  //   transform.position_dxdy.y += sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  // }
  // if (get_key_held(input, SDL_SCANCODE_DOWN)) {
  //   transform.position_dxdy.x += -sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  //   transform.position_dxdy.y += cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  // } else if (get_key_held(input, SDL_SCANCODE_UP)) {
  //   transform.position_dxdy.x -= -sin(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  //   transform.position_dxdy.y -= cos(glm::radians(camera.rotation)) * CAM_SPEED * dt;
  // }

  // int dx = static_cast<int>(transform.position_dxdy.x);
  // if (glm::abs(dx) > 0) {
  //   transform.position.x += dx;
  //   transform.position_dxdy.x -= dx;
  // }
  // int dy = static_cast<int>(transform.position_dxdy.y);
  // if (glm::abs(dy) > 0) {
  //   transform.position.y += dy;
  //   transform.position_dxdy.y -= dy;
  // }

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

} // namespace game2d