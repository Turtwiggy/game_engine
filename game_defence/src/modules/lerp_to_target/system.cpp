#include "system.hpp"

#include "components.hpp"
#include "modules/renderer/components.hpp"
#include <glm/gtx/compatibility.hpp> // lerp
#include "modules/lifecycle/components.hpp"

namespace game2d {

void
update_lerp_to_target_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const float dt = milliseconds_dt / 1000.0f;

  // lerp to position
  const auto& view = r.view<TransformComponent, const HasTargetPositionComponent, const LerpToTargetComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, target_t, lerp] : view.each()) {
    const glm::vec2 start = { transform.position.x, transform.position.y };
    const glm::vec2 end = target_t.position;

    // note: this lerp would not work in an update() function where dt is variable
    // const float t = dt * bow.lerp_speed;
    const float t = 1.0f - glm::pow(0.5f, dt * lerp.speed);
    const glm::vec2 new_pos = glm::lerp(start, end, t);

    transform.position.x = new_pos.x;
    transform.position.y = new_pos.y;
  }
};

} // namespace game2d