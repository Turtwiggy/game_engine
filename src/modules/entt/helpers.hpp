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

template<class T>
void
ctx_reset(entt::registry& r)
{
  if (r.ctx().contains<T>())
    r.ctx().erase<T>();
  r.ctx().emplace<T>();
};

}; // namespace game2d
