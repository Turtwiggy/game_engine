#include "settings.hpp"

#include <nlohmann/json.hpp>

#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include <string>


namespace game2d {

void
save_string(const std::string& key, const std::string& value)
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
  data[key] = value;

  // save data
  std::ofstream fout(SETTINGS_FILEPATH);
  fout << data;
};

std::optional<std::string>
get_string(const std::string& key)
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

  const nlohmann::json data = nlohmann::json::parse(data_str);
  if (data.contains(key))
    return data[key];
  return std::nullopt;
};

std::string
gesert_string(const std::string& key, const std::string& value)
{
  const auto str = get_string(key);
  if (str == std::nullopt) {
    save_string(key, value);
    return value;
  }
  return str.value();
};

} // namespace game2d