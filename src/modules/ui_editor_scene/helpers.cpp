#include "helpers.hpp"

#include "serialize.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace game2d {

void
save(const entt::registry& registry, std::string path)
{
  std::cout << "saving..."
            << "\n";

  // // convert entt registry to string(or bson)
  // NJSONOutputArchive json_archive;
  // entt::basic_snapshot{ registry }.entities(json_archive).component<Plant>(json_archive);
  // json_archive.close();

  // std::string data = json_archive.as_string();

  // // save to disk
  // std::ofstream fout(path);
  // fout << data.c_str();
};

void
load(entt::registry& registry, std::string path)
{
  std::cout << "loading..."
            << "\n";

  // registry.clear();

  // // load from disk
  // std::ifstream t(path);
  // std::stringstream buffer;
  // buffer << t.rdbuf();
  // const std::string data = buffer.str();

  // // convert string (or bson) to entt registry
  // auto& registry_to_load_in_to = registry;

  // NJSONInputArchive json_in(data);
  // entt::basic_snapshot_loader{ registry_to_load_in_to }.entities(json_in).component<Plant>(json_in);
};

void
load_if_exists(entt::registry& registry, std::string path){
  //   std::ifstream file(path.c_str());
  //   if (file)
  //     load(registry, path);
};

} // namespace game2d