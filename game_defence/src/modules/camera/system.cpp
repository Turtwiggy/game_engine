// header
#include "system.hpp"

// components/systems
#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/hearth/components.hpp"
#include "modules/player/components.hpp"
#include "renderer/components.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

namespace game2d {

void
update_camera_system(entt::registry& r, float dt)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto camera_ent = get_first<OrthographicCamera>(r);

  // camera to follow a target
  const auto& target_view = r.view<HearthComponent>();
  if (target_view.size() == 0)
    return; // no target to home in on
  const auto target_entity = target_view.front();
  const auto& target_transform = r.get<TransformComponent>(target_entity);

  auto& camera = r.get<OrthographicCamera>(camera_ent);
  auto& camera_transform = r.get<TransformComponent>(camera_ent);
  camera_transform.scale.x = 1;
  camera_transform.scale.y = 1;

  static bool done_first_time = false;
  if (!done_first_time) {
    done_first_time = true;
    camera_transform.position = target_transform.position;
  }

  if (get_key_down(input, SDL_SCANCODE_RETURN))
    camera_transform.position = target_transform.position;

  float CAM_SPEED = 500.0f;
  if (get_key_held(input, SDL_SCANCODE_LSHIFT))
    CAM_SPEED *= 2.0f;

  const int mul = static_cast<int>(CAM_SPEED * dt);

  if (get_key_held(input, SDL_SCANCODE_LEFT))
    camera_transform.position.x -= mul;
  if (get_key_held(input, SDL_SCANCODE_RIGHT))
    camera_transform.position.x += mul;
  if (get_key_held(input, SDL_SCANCODE_UP))
    camera_transform.position.y -= mul;
  if (get_key_held(input, SDL_SCANCODE_DOWN))
    camera_transform.position.y += mul;

  //   glm::vec3 lerp_pos = glm::lerp(pos_as_vec3, target_position, glm::clamp(dt * damping, 0.0f, 1.0f));

  TransformComponent offset_transform = camera_transform;
  offset_transform.position.x = static_cast<int>(-ri.viewport_size_render_at.x / 2.0f) + camera_transform.position.x;
  offset_transform.position.y = static_cast<int>(-ri.viewport_size_render_at.y / 2.0f) + camera_transform.position.y;

  camera.view = calculate_ortho_view(offset_transform);

  // if (get_mouse_mmb_held()) { // pan
  // if (get_mouse_lmb_held()) { // rotate
  // if (get_mouse_rmb_held()) { // zoom
};

} // namespace game2d