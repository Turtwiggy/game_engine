#include "system.hpp"

#include "components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_lerp_to_target_system(entt::registry& r, const float& dt)
{
  // lerp to position
  const auto& view = r.view<TransformComponent, const HasTargetPositionComponent, const LerpToTargetComponent>(
    entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, target_t, lerp] : view.each()) {
    const glm::vec2 start = { transform.position.x, transform.position.y };
    const glm::vec2 end = target_t.position;

    const float t = 1.0f - glm::pow(0.5f, dt * lerp.speed);
    const glm::vec2 new_pos = glm::lerp(start, end, t);
    transform.position = { new_pos.x, new_pos.y, 0.0f };

    //
  }
};

} // namespace game2d