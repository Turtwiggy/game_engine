#pragma once

#include <entt/entt.hpp>

#include <optional>
#include <utility>

namespace game2d {

template<class T>
entt::entity
get_first(entt::registry& r)
{
  const auto& view = r.view<T>();
  if (view.size() == 0)
    return entt::null;
  return view.front();
};

template<class T>
T&
get_first_component(entt::registry& r)
{
  const auto& e = get_first<T>(r);
  return r.get<T>(e);
};

}; // namespace game2d
