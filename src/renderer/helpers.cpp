#include "helpers.hpp"

#include "renderer/transform.hpp"

namespace game2d {

void
set_position(entt::registry& r, const entt::entity& e, const glm::vec3& position)
{
  r.get<TransformComponent>(e).position = position;
};

} // namespace game2d