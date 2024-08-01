#pragma once

#include <entt/entt.hpp>

namespace game2d {

template<class A, class B>
std::pair<entt::entity, entt::entity>
collision_of_interest(const entt::registry& r, const entt::entity& a, const entt::entity& b)
{
  {
    const auto* a_has_type_a = r.try_get<A>(a);
    const auto* b_has_type_b = r.try_get<B>(b);
    if (a_has_type_a != nullptr && b_has_type_b != nullptr)
      return { a, b };
  }
  {
    const auto* a_has_type_b = r.try_get<B>(a);
    const auto* b_has_type_a = r.try_get<A>(b);
    if (a_has_type_b != nullptr && b_has_type_a != nullptr)
      return { b, a };
  }
  return { entt::null, entt::null };
};

} // namespace game2d