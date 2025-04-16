#pragma once

#include <entt/fwd.hpp>

#include <unordered_set>
#include <vector>

namespace game2d {

template<class T>
bool
has(const std::vector<T>& vec, const T& el)
{
  return std::find(vec.begin(), vec.end(), el) != vec.end();
};

template<class T>
bool
has(const std::unordered_set<T>& vec, const T& el)
{
  return std::find(vec.begin(), vec.end(), el) != vec.end();
};

} // namespace game2d