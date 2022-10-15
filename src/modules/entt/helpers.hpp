#pragma once

#include <entt/entt.hpp>

#include <optional>
#include <utility>

namespace game2d {

template<class T>
std::optional<std::reference_wrapper<T>>
get_first(entt::registry& r)
{
  const auto& view = r.view<T>();
  if (view.size() == 0)
    return std::nullopt;

  return r.get<T>(view.front());
};

}; // namespace game2d
