#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "io_components.hpp"
#include "io_helpers.hpp"
#include <nlohmann/json.hpp>

namespace game2d {

const std::string SETTINGS_FILEPATH = "save.json";

// load from disk
SINGLE_OnDiskData
savefile_load_disk(entt::registry& r)
{
  if (!std::filesystem::exists(SETTINGS_FILEPATH)) {
    std::ofstream fout(SETTINGS_FILEPATH);
    fout << "{}";
  }

  // load from disk
  std::ifstream t(SETTINGS_FILEPATH);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_str = buffer.str();

  // convert to struct
  nlohmann::json root = nlohmann::json::parse(data_str);
  auto data = root.get<SINGLE_OnDiskData>();
  return data;
};

// save back to disk
void
savefile_save_disk(entt::registry& r)
{
  const auto disk_e = get_first<SINGLE_OnDiskData>(r);
  if (disk_e == entt::null) {
    SDL_Log("update_save() failed, no disk_e");
    return;
  }
  const auto& disk_c = r.get<SINGLE_OnDiskData>(disk_e);

  if (!std::filesystem::exists(SETTINGS_FILEPATH)) {
    std::ofstream fout(SETTINGS_FILEPATH);
    fout << "{}";
  }

  // convert struct to string.
  nlohmann::json data = disk_c;

  // save data
  std::ofstream fout(SETTINGS_FILEPATH);
  fout << data;

  SDL_Log("game saved to disk.");
}

std::optional<nlohmann::json>
savefile_get_key(entt::registry& r, const std::string& str)
{
  const auto disk_c = get_first_component<SINGLE_OnDiskData>(r);
  if (disk_c.data.contains(str))
    return disk_c.data.at(str);
  return std::nullopt;
};

void
savefile_put_key(entt::registry& r, const std::string& key, const nlohmann::json& val)
{
  auto& disk_c = get_first_component<SINGLE_OnDiskData>(r);
  disk_c.data[key] = val;
};

} // namespace game2d