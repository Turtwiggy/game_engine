#pragma once

#include <entt/entt.hpp>
#include <optional>

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
T&
destroy_and_create(entt::registry& r, const std::optional<T> val = std::nullopt)
{
  const entt::entity ent = get_first<T>(r);
  if (ent != entt::null)
    r.destroy(ent);

  if (val.has_value())
    return r.emplace<T>(r.create(), val.value());
  return r.emplace<T>(r.create());
};

}; // namespace game2d
