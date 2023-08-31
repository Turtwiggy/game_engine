#pragma once

#include <entt/entt.hpp>

#include <optional>
#include <utility>

namespace game2d {

template<class T>
[[nodiscard]] entt::entity
get_first(entt::registry& r)
{
  return r.view<const T>().front();
};

template<class T>
[[nodiscard]] T&
get_first_component(entt::registry& r)
{
  const auto e = get_first<T>(r);
  return r.get<T>(e);
};

template<class T>
void
destroy_and_create(entt::registry& r)
{
  const entt::entity ent = get_first<T>(r);
  if (ent != entt::null)
    r.destroy(ent);
  r.emplace<T>(r.create());
};

}; // namespace game2d
