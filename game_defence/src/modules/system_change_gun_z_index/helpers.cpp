#include "helpers.hpp"

#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
set_z_index(entt::registry& r, const entt::entity e, const ZLayer& layer)
{
  r.get<ZIndex>(e).layer = layer;
};

} // namespace game2d