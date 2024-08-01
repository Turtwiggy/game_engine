#pragma once

#include "renderer/components.hpp"
#include <entt/entt.hpp>

#include <optional>
#include <string>

namespace game2d {

template<class T>
entt::entity
create_empty(entt::registry& r, const std::optional<T>& val = std::nullopt)
{
  // val passed in. could be useful for debugging

  const T t{};
  const std::string name = typeid(t).name();

  const auto e = r.create();
  r.emplace<TagComponent>(e, name);

  if (val.has_value())
    r.emplace<T>(e, val.value());
  else
    r.emplace<T>(e); // default

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
entt::entity
destroy_first_and_create(entt::registry& r, const std::optional<T> val = std::nullopt)
{
  destroy_first<T>(r, val);

  return create_empty<T>(r, val);
};

template<class T>
bool // existed
remove_if_exists(entt::registry& r, const entt::entity e)
{
  if (const auto* comp = r.try_get<T>(e)) {
    r.remove<T>(e);
    return true;
  }
  return false;
};

}; // namespace game2d
