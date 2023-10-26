// header
#include "system.hpp"

// components/systems
#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

namespace game2d {

void
update_camera_system(entt::registry& r, const float dt)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  const auto camera_ent = get_first<OrthographicCamera>(r);

  auto& camera = r.get<OrthographicCamera>(camera_ent);
  auto& camera_transform = r.get<TransformComponent>(camera_ent);
  TransformComponent offset_transform = camera_transform;
  offset_transform.position.x = static_cast<int>(-ri.viewport_size_render_at.x / 2.0f) + camera_transform.position.x;
  offset_transform.position.y = static_cast<int>(-ri.viewport_size_render_at.y / 2.0f) + camera_transform.position.y;
  camera.view = calculate_ortho_view(offset_transform);

  // calculate center of all targets
  auto target_position = camera_transform.position;
  const auto& targets_view = r.view<CameraFollow, TransformComponent>();
  for (const auto& [entity, follow, target_transform] : targets_view.each()) {
    target_position.x += target_transform.position.x;
    target_position.x /= 2.0f;
    target_position.y += target_transform.position.y;
    target_position.y /= 2.0f;
  }

  // update lerp
  // if (auto* target = r.try_get<HasTargetPositionComponent>(camera_ent))
  //   target->position = { target_position.x, target_position.y };

  const glm::ivec2 allowed_bounds{ 100, 100 };
  const glm::ivec2 offset = allowed_bounds;
  // camera_transform.position = target_position;

  const float CAM_SPEED = 500.0f;
  const int mul = static_cast<int>(CAM_SPEED * dt);
  if (get_key_held(input, SDL_SCANCODE_LEFT))
    camera_transform.position.x -= mul;
  if (get_key_held(input, SDL_SCANCODE_RIGHT))
    camera_transform.position.x += mul;
  if (get_key_held(input, SDL_SCANCODE_UP))
    camera_transform.position.y -= mul;
  if (get_key_held(input, SDL_SCANCODE_DOWN))
    camera_transform.position.y += mul;

  // if (get_mouse_mmb_held()) { // pan
  // if (get_mouse_lmb_held()) { // rotate
  // if (get_mouse_rmb_held()) { // zoom
};

} // namespace game2d