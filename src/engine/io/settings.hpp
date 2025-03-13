#pragma once

#include <SDL2/SDL_log.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <fstream>
#include <optional>
#include <sstream>
#include <string>

namespace game2d {

const std::string SETTINGS_FILEPATH = "save.json";
const std::string PLAYERPREF_MUTE = "playerpref_mute";
const std::string SPACESHIP_COUNT = "spaceship_count";

void
save_string(const std::string& key, const std::string& value);

[[nodiscard]] std::optional<std::string>
get_string(const std::string& key);

[[nodiscard]] std::string
gesert_string(const std::string& key, const std::string& default_value);

//

template<class T>
void
save_vector(const std::string& key, const std::vector<T>& val)
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

  // update data
  nlohmann::json data = nlohmann::json::parse(data_str);
  data[key] = val;

  // save data
  std::ofstream fout(SETTINGS_FILEPATH);
  fout << data;
};

template<class T>
[[nodiscard]]
std::optional<std::vector<T>>
get_vector(const std::string& key)
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

  // check the vector key exists
  const nlohmann::json data = nlohmann::json::parse(data_str);
  if (!data.contains(key)) {
    SDL_Log("Warning: missing key %s for get_vector", key.c_str());
    return std::nullopt;
  }

  std::vector<T> result;

  for (const auto& val : data[key]) {

    T game_ver_data;
    const int disk_ver = val.at("version");
    const int game_ver = game_ver_data.version;

    if (disk_ver != game_ver)
      SDL_Log("Serialization mismatch... (disk)%i (game)%i", disk_ver, game_ver);

    if (disk_ver > game_ver) {
      // somehow,
      // data on disk is a higher version.
      // maybe you're playing an old version of the game?
      SDL_Log("Old game with new save file; probably losing some data...");
    }

    if (game_ver > disk_ver) {
      // you likely updated your game,
      // and there's new features and such,
      // and your save file needs to be updated.
      SDL_Log("Updating older version save file...");
    }

    // easy case: data versions match
    if (disk_ver == game_ver) {
      // all good
    }

    //
    // note: missing values given defaults.
    // note: unexpected values are ignored.
    //
    T on_disk_data;
    val.get_to<T>(on_disk_data);
    on_disk_data.version = game_ver;
    result.push_back(on_disk_data);
  }

  return result;

  /*
  try {
    if (data.contains(key)) {
      return data[key];
    }
  } catch (const nlohmann::json::parse_error& e) {
    SDL_Log("JSON parse error: %s", e.what());
  } catch (const nlohmann::json::type_error& e) {
    SDL_Log("JSON type error: %s", e.what());
    SDL_Log("Data stored on disk likely corrupt or unexpected format...");
  } catch (const std::exception& e) {
    SDL_Log("General error: %s", e.what());
  }
  */

  return std::nullopt;
};

} // namespace game2d