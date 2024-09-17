#include "helpers.hpp"

#include "engine/renderer/transform.hpp"
#include "modules/renderer/components.hpp"


namespace game2d {

void
set_z_index(entt::registry& r, const entt::entity e, const ZLayer& layer)
{
  r.get<TransformComponent>(e).z_index = static_cast<int>(layer);
};

} // namespace game2d