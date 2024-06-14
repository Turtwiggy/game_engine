#pragma once

#include "renderer/components.hpp"
#include <entt/entt.hpp>

#include <optional>
#include <string>

namespace game2d {

template<class T>
[[nodiscard]] entt::entity
create_empty(entt::registry& r, const std::optional<T> val = std::nullopt)
{
  // val passed in. could be useful for debugging

  const T t{};
  const std::string name = typeid(t).name();

  const auto e = r.create();
  r.emplace<TagComponent>(e, name);

  return e;
}

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
destroy_first(entt::registry& r, const std::optional<T> val = std::nullopt)
{
  const entt::entity ent = get_first<T>(r);
  if (ent != entt::null)
    r.destroy(ent);
};

template<class T>
T&
destroy_first_and_create(entt::registry& r, const std::optional<T> val = std::nullopt)
{
  destroy_first<T>(r, val);

  if (val.has_value())
    return r.emplace<T>(create_empty<T>(r, val.value()), val.value());

  return r.emplace<T>(create_empty<T>(r));
};

}; // namespace game2d
