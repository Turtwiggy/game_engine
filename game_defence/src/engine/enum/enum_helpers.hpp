#pragma once

#include <entt/entt.hpp>
#include <magic_enum.hpp>

namespace engine {

template<typename T>
std::string
convert_enum_to_string(const T& t)
{
  return std::string(magic_enum::enum_name(t));
};

template<typename T>
std::vector<std::string>
enum_class_to_vec_str()
{
  std::vector<std::string> result;
  for (int i = 0; i < static_cast<int>(T::count); i++) {
    const T hmm = static_cast<T>(i);
    result.push_back(convert_enum_to_string(hmm));
  }

  return result;
}

} // namespace game2d