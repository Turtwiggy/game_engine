#include "helpers.hpp"

#include "lifecycle/components.hpp"
#include "modules/actor_weapon/components.hpp"

namespace game2d {

entt::entity
get_gun(entt::registry& r, const entt::entity& parent)
{
  for (const auto& [e, parent_c, cooldown] : r.view<HasParentComponent, WeaponComponent>().each())
    if (parent_c.parent == parent)
      return e;
  return entt::null;
};

} // namespace game2d