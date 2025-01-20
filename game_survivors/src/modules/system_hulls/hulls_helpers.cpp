#include "hulls_helpers.hpp"

#include "hulls_components.hpp"

#include <SDL2/SDL_log.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace game2d {

ShipHullData
load_hull(std::string filepath)
{
  // load from disk
  std::ifstream t(filepath);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = line.find("//") != std::string::npos ? "" : line;
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);

  return root.get<ShipHullData>();
};

// directory e.g. assets/raws/hulls/
SINGLE_Hulls
load_hulls(std::string directory)
{
  SINGLE_Hulls hulls;

  for (const auto& dir : std::filesystem::recursive_directory_iterator(directory)) {
    if (!dir.is_regular_file())
      continue;

    const auto filename = dir.path().filename().string();
    const auto extension = dir.path().extension().string();

    if (extension != ".jsonc")
      continue;

    auto filepath = dir.path().string();
    SDL_Log("loading hull... %s", filepath.c_str());

    // load the hull
    hulls.hulls.push_back(load_hull(filepath));
  }

  return hulls;
};

std::optional<ShipHullData>
get_hull(const SINGLE_Hulls& hulls, const std::string& hull_name)
{
  for (const auto& h : hulls.hulls) {
    if (h.name == hull_name)
      return h;
  }
  return std::nullopt;
};

} // namespace game2d