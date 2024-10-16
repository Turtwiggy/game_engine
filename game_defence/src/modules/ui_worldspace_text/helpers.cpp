#include "modules/ui_worldspace_text/helpers.hpp"

namespace game2d {

std::vector<std::string>
split_string_nearest_space(const std::string& str, const int& len)
{
  std::vector<std::string> results;

  size_t start = 0;
  while (start < str.size()) {
    size_t end = start + len;

    // Get the nearest ' ' char to the end of the sentence
    if (end < str.size() && str[end] != ' ') {
      const size_t space_pos = str.rfind(' ', end);
      if (space_pos != std::string::npos && space_pos > start)
        end = (int)space_pos;
    }

    // add the sentence so far
    results.push_back(str.substr(start, end - start));

    // onwards..!
    start = end;
    while (start < str.size() && str[start] == ' ')
      ++start; // Skip spaces
  }

  return results;
};

} // namespace game2d