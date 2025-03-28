#include "pch.hpp"

#include "engine/lifecycle/components.hpp"
#include "lifecycle_helpers.hpp"

namespace game2d {

bool
is_parent_valid(entt::registry& r, entt::entity e)
{
  auto* parent_c = r.try_get<HasParentComponent>(e);
  if (!parent_c)
    return false;

  if (!r.valid(parent_c->parent) || parent_c->parent == entt::null)
    return false;

  return true;
};

} // namespace game2d