#include "helpers.hpp"

#include "renderer/transform.hpp"

namespace game2d {

void
set_z_index(entt::registry& r, const entt::entity e, const int index)
{
  r.get<TransformComponent>(e).position.z = index;
};

} // namespace game2d