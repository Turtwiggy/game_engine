#pragma once

#include <entt/fwd.hpp>

namespace game2d {

template<class A, class B>
std::pair<entt::entity, entt::entity>
coll(entt::registry& r, entt::entity a, entt::entity b)
{
  {
    const auto* a_has_type_a = r.try_get<A>(a);
    const auto* b_has_type_b = r.try_get<B>(b);
    if (a_has_type_a && b_has_type_b)
      return { a, b };
  }
  {
    const auto* a_has_type_b = r.try_get<B>(a);
    const auto* b_has_type_a = r.try_get<A>(b);
    if (a_has_type_b && b_has_type_a)
      return { b, a };
  }
  return { entt::null, entt::null };
};

struct OnCollisionEnter
{
  entt::entity a = entt::null;
  entt::entity b = entt::null;
};

struct OnCollisionExit
{
  entt::entity a = entt::null;
  entt::entity b = entt::null;
};

} // namespace game2d