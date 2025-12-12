#include "pch.hpp"

#include "hulls_helpers.hpp"

#include "hulls_components.hpp"
#include "modules/core/raws/raws_helpers.hpp"

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
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);

  return root.get<ShipHullData>();
};

// directory e.g. assets/raws/hulls/
SINGLE_Hulls
load_hulls(entt::registry& r, std::string directory)
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

  if (hulls.hulls.empty()) {
    throw std::runtime_error("No hulls in SINGLE_Hulls()");
    exit(1); // crash
  }

  // Sort em
  auto sorted_hulls = hulls.hulls;
  // auto sort_by_hullsize = [](const ShipHullData& a, const ShipHullData& b) {
  //   const int size_a = a.height * a.width;
  //   const int size_b = b.height * b.width;
  //   return size_a < size_b;
  // };
  auto sort_by_hardpoints_then_size = [](const ShipHullData& a, const ShipHullData& b) {
    const int hardpoints_a = (int)a.hardpoints.size();
    const int hardpoints_b = (int)b.hardpoints.size();
    if (hardpoints_a != hardpoints_b)
      return hardpoints_a < hardpoints_b;

    const int size_a = a.height * a.width;
    const int size_b = b.height * b.width;
    return size_a < size_b;
  };
  std::sort(sorted_hulls.begin(), sorted_hulls.end(), sort_by_hardpoints_then_size);

  hulls.hulls = sorted_hulls;
  return hulls;
};

} // namespace game2d