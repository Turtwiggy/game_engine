#include "pch.hpp"

#include "raws_helpers.hpp"

namespace game2d {

std::string
remove_comment(const std::string& str)
{
  const auto pos = str.find("//");
  if (pos != std::string::npos)
    return str.substr(0, pos);
  return str;
};

} // namespace game2d