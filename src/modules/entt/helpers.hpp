#pragma once

#include <entt/entt.hpp>

namespace game2d {

template<class T>
T&
get_first(entt::registry& r)
{
  const entt::entity entity = r.view<T>().front();
  return r.get<T>(entity);
};

}; // namespace game2d
