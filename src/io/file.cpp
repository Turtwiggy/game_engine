#include "file.hpp"

#include <fstream>

namespace game2d {

std::vector<std::string>
load_file_into_lines(const std::string& path)
{
  std::vector<std::string> results;

  std::ifstream file(path);
  if (!file.is_open())
    return results;

  std::string line;
  while (std::getline(file, line))
    results.push_back(line);

  file.close();

  return results;
}

} // namespace game2d