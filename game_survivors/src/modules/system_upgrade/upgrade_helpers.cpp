#include "upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_raws/raws_helpers.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include <SDL2/SDL_log.h>
#include <magic_enum.hpp>

#include <format>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace game2d {

SINGLE_Upgrades
load_upgrades(std::string path)
{
  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    const std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }
  const std::string string_without_comments = output.str();

  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  const auto upgrades_c = root.get<SINGLE_Upgrades>();

  // Parse the upgrades at load, to make sure they're all valid.
  for (const Upgrade& u : upgrades_c.upgrades) {
    const auto& effects = u.effects;

    for (const Effects& effect : effects) {

      // Validate UpgradeableStat
      if (effect.stat.has_value()) {
        const auto& stat_raw = effect.stat.value();
        const auto stat_enum_opt = magic_enum::enum_cast<UpgradeableStat>(stat_raw);
        if (!stat_enum_opt.has_value()) {
          std::string err = std::format("Unknown stat: {}", stat_raw);
          SDL_Log("%s", err.c_str());
          exit(1); // crash
        }
        const auto stat_enum = stat_enum_opt.value();
        const auto stat_enum_key = std::string(magic_enum::enum_name(stat_enum));
      }

      // Validate Traits
      if (effect.trait.has_value()) {
        const auto& trait_raw = effect.trait.value();
        const auto trait_enum_opt = magic_enum::enum_cast<AquirableTrait>(trait_raw);
        if (!trait_enum_opt.has_value()) {
          std::string err = std::format("Unknown trait: {}", trait_raw);
          SDL_Log("%s", err.c_str());
          exit(1); // crash
        }
        const auto trait_enum = trait_enum_opt.value();
        const auto trait_enum_key = std::string(magic_enum::enum_name(trait_enum));
      }
    }
  }

  return upgrades_c;
};

std::vector<std::string>
available_upgrade_names(entt::registry& r)
{
  const auto& up_c = get_first_component<SINGLE_Upgrades>(r);

  std::vector<std::string> keys;
  const auto get_upgrade_names = [](const auto& u) { return u.name; };
  std::transform(up_c.upgrades.begin(), up_c.upgrades.end(), std::back_inserter(keys), get_upgrade_names);

  return keys;
};

Upgrade
find_upgrade(entt::registry& r, const std::string& key)
{
  const auto& up_c = get_first_component<SINGLE_Upgrades>(r);

  auto find_lambda = [&key](const Upgrade& u) { return u.name == key; };
  auto it = std::find_if(up_c.upgrades.begin(), up_c.upgrades.end(), find_lambda);

  if (it == up_c.upgrades.end()) {
    SDL_Log("Oops! No upgrade found %s", key.c_str());
    exit(1); // crash
  }

  return *it;
};

std::string
generate_description(const Upgrade& u)
{
  if (u.desc.has_value())
    return u.desc.value();

  std::string desc = "";

  for (const Effects& eff : u.effects) {
    if (!eff.stat.has_value())
      continue;
    const auto& type = eff.type.value();
    const auto& val_json = eff.value.value();

    float val_float = 0.0f;
    if (val_json.is_number())
      val_json.get_to(val_float);

    const auto float_to_str = [](float num, int dp) -> std::string {
      std::ostringstream oss;
      oss << std::fixed << std::setprecision(dp) << num;
      return oss.str();
    };

    // Generate a description
    desc += eff.stat.value();
    desc += " +";
    desc += float_to_str(val_float, 0);
    if (type == "stat_percent_increase")
      desc += "%";
    desc += " ";
  }

  return desc;
}

} // namespace game2d