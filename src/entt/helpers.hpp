#pragma once

#include "renderer/components.hpp"
#include <entt/entt.hpp>

#include <fmt/core.h>
#include <format>
#include <optional>
#include <string>

namespace game2d {

inline std::string
cleanup_tag_str(const std::string& s)
{
  // cleanup the generated tag...
  std::string cleanedup_tag = s;
  const std::string search_str = "struct game2d::";
  const std::size_t pos = cleanedup_tag.find(search_str);
  if (pos != std::string::npos)
    cleanedup_tag = cleanedup_tag.substr(pos + search_str.length());
  return cleanedup_tag;
};

template<class T>
entt::entity
create_empty(entt::registry& r, const std::optional<T>& val = std::nullopt)
{
  // val passed in. could be useful for debugging

  const T t{};
  const std::string name = typeid(t).name();
  const std::string tag = cleanup_tag_str(name);

  const auto e = r.create();
  r.emplace<TagComponent>(e, tag);

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

  if (e == entt::null) {
    const T t{};
    const std::string name = typeid(t).name();
    const std::string err = std::format("get_first_component<{}>() missing", name);
    fmt::println("Error: {}", err);
    throw std::runtime_error(err);
    exit(1); // crash
  }

  return r.get<T>(e);
};

template<class T>
void
destroy_first(entt::registry& r)
{
  const entt::entity ent = get_first<T>(r);
  if (ent != entt::null)
    r.destroy(ent);
};

template<class T>
entt::entity
destroy_first_and_create(entt::registry& r, const std::optional<T> val = std::nullopt)
{
  destroy_first<T>(r);

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
