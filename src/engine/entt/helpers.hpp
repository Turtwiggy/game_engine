#pragma once

#include "engine/renderer/transform.hpp"

#include <SDL2/SDL_log.h>
#include <entt/entt.hpp>

#include <format>
#include <optional>
#include <string>

namespace game2d {

// i.e. not to removed between scenes (without good reason)
struct Persistent
{
  bool placeholder = true;
};

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

  // this error can also be caused by attempting to use typeid
  // on an object of a class that has no virtual functions.
  // C++ RTTI requires a vtable, so classes that you wish to perform
  // type identification on require at least one virtual function.
  // If you want type information to work on a class for which you don't
  // really want any virtual functions, make the destructor virtual.
  const std::string name = typeid(T).name();
  const std::string tag = cleanup_tag_str(name);
  // const std::string tag = "empty";

  const auto e = r.create();
  r.emplace<TagComponent>(e, tag);

  if (val.has_value())
    r.emplace<T>(e, val.value());
  else
    r.emplace<T>(e); // default

  return e;
}

template<class T>
entt::entity
create_persistent(entt::registry& r, const std::optional<T>& val = std::nullopt)
{
  auto e = create_empty(r, val);
  r.emplace<Persistent>(e);
  return e;
};

template<class T, typename... Other>
[[nodiscard]] entt::entity
get_first(entt::registry& r)
{
  return r.view<const T, const Other...>().front();
};

template<class T>
[[nodiscard]] T&
get_first_component(entt::registry& r)
{
  const auto e = get_first<T>(r);

  if (e == entt::null) {
    const std::string name = typeid(T).name();
    // std::string name = "unknown";
    const std::string err = std::format("get_first_component<{}>() missing", name);
    SDL_Log("%s", std::format("Error: {}", err).c_str());
    throw std::runtime_error(err);
    exit(1); // crash
  }

  auto& ref = r.get<T>(e);
  return ref;
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

// get or insert
template<class T>
T&
gesert_component(entt::registry& r)
{
  if (get_first<T>(r) == entt::null) {
    auto e = create_empty<T>(r);
    return r.get<T>(e);
  }
  return get_first_component<T>(r);
};

template<typename View, typename Predicate>
std::vector<entt::entity>
filter_view(View view, Predicate pred)
{
  std::vector<entt::entity> result;
  for (auto entity : view)
    if (pred(entity))
      result.push_back(entity);
  return result;
};

#define GET_FIRST_OR_RETURN(TYPE, REGISTRY, ENTITY_VAR, COMPONENT_VAR)                                                      \
  const auto ENTITY_VAR = get_first<TYPE>(REGISTRY);                                                                        \
  if (ENTITY_VAR == entt::null)                                                                                             \
    return;                                                                                                                 \
  auto& COMPONENT_VAR = r.get<TYPE>(ENTITY_VAR);

}; // namespace game2d
