#include "pch.hpp"

#include "persistent_upgrade_components.hpp"
#include "persistent_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "modules/systems/system_traits/trait_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"

namespace game2d {

struct StatInfo
{
  std::string key;
  std::string type;
  float val;
};

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

    // validate key
    auto key = u.key;
    const auto stat_enum_opt = magic_enum::enum_cast<UpgradeableStat>(key);
    if (!stat_enum_opt.has_value()) {
      std::string err = std::format("Unknown stat: {}", key);
      SDL_Log("%s", err.c_str());
      exit(1); // crash
    }
  }

  return upgrades_c;
};

std::vector<std::string>
available_upgrade_names(entt::registry& r)
{
  const auto& up_c = get_first_component<SINGLE_Upgrades>(r);

  std::vector<std::string> keys;
  const auto get_upgrade_names = [](const auto& u) { return u.key; };
  std::transform(up_c.upgrades.begin(), up_c.upgrades.end(), std::back_inserter(keys), get_upgrade_names);

  return keys;
};

Upgrade
find_upgrade(entt::registry& r, const std::string& key)
{
  const auto& up_c = get_first_component<SINGLE_Upgrades>(r);

  auto find_lambda = [&key](const Upgrade& u) { return u.key == key; };
  auto it = std::find_if(up_c.upgrades.begin(), up_c.upgrades.end(), find_lambda);

  if (it == up_c.upgrades.end()) {
    SDL_Log("Oops! No upgrade found %s", key.c_str());
    exit(1); // crash
  }

  return *it;
};

void
load_persistent_upgrades_and_apply_to_player(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, upgrade_e, upgrade_c);

  std::vector<StatInfo> stats;

  for (int i = 0; i < (int)UpgradeableStat::count; i++) {
    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    const auto val_opt = savefile_get_key(r, stat_str);
    if (!val_opt.has_value())
      continue;
    const auto val_json = val_opt.value();
    SDL_Log("You've purchased an upgrade... %s", stat_str.c_str());

    // Load how much the stat gives from the upgrades.
    const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
    const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
    if (it == upgrade_c.upgrades.end()) {
      // You've purchased an upgrade that isnt in the SINGLE_Upgrades.upgrades. CURIOUS
      continue;
    }

    const Upgrade u = (*it);

    int your_level = 0;
    val_json.get_to<int>(your_level);

    for (int lv_idx = 0; lv_idx < your_level; lv_idx++) {
      auto lv_info = u.levels[lv_idx];

      StatInfo stat_info;
      stat_info.key = u.key;
      stat_info.type = lv_info.type; // stat or flat percent
      stat_info.val = lv_info.value;
      stats.push_back(stat_info);
    }
  }

  for (const auto& [e, player_c, stat_c] : r.view<const PlayerComponent, StatModifierComponent>().each()) {
    // add to stats

    for (const auto& stat_info : stats) {

      const bool add_flat = stat_info.type == "stat_flat_increase";
      const bool add_percent = stat_info.type == "stat_percent_increase";
      const auto mod_key = stat_info.key;
      const auto mod_val = stat_info.val;

      if (add_flat)
        stat_c.add(std::make_shared<StatFlatIncrease>(mod_val, mod_key));
      else if (add_percent)
        stat_c.add(std::make_shared<StatPercentIncrease>(mod_val, mod_key));

      //
    }
  }
}

/*
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
*/

} // namespace game2d