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

}; // namespace game2d
