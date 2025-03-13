#pragma once

#include <entt/fwd.hpp>

#include <vector>

namespace game2d {

template<class T>
std::vector<entt::entity>
view_to_vector_of_ents(entt::registry& r)
{
  auto view = r.view<T>();

  std::vector<entt::entity> vec;
  for (const auto& [e, comp_c] : view.each())
    vec.push_back(e);

  return vec;
};

} // namespace game2d