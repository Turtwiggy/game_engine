#pragma once

#include <entt/fwd.hpp>

#include <optional>
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

template<class T>
std::optional<int>
index_of(const std::vector<T>& vec, const T& el)
{
  for (int i = 0; i < vec.size(); i++)
    if (vec[i] == el)
      return i;
  return std::nullopt;
};

} // namespace game2d