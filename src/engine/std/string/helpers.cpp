#include "pch.hpp"

#include "helpers.hpp"

namespace game2d {

std::string
to_upper(const std::string& str)
{
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::toupper(c); });
  return result;
};

std::string
to_lower(const std::string& str)
{
  std::string result = str;
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
  return result;
};

std::string
str_remove_all_occurances(std::string base, const std::string& substr)
{
  size_t pos = 0;
  while ((pos = base.find(substr, pos)) != std::string::npos)
    base.erase(pos, substr.length());
  return base;
};

} // namespace game2d