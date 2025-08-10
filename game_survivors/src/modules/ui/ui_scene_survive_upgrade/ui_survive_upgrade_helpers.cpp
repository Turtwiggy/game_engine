#include "pch.hpp"

#include "ui_survive_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/std/string/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "ui_survive_upgrade_components.hpp"

namespace game2d {

#if defined(_DEBUG)
// static engine::RandomState roll_rnd(0); // same roll every time
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#else
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#endif

void
generate_upgrades_for_players(entt::registry& r, SINGLE_LevelUpUI& ui_c)
{
  int num_players = 4;

  for (int i = 0; i < num_players; i++) {
    auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;
    const auto& player_c = r.get<const PlayerBoatComponent>(player_e);

    UpgradeResultsComponent results_c;

    const auto get_rarity_from_roll = [](float roll) -> Rarity {
      int sum = 0;
      for (const auto [type, value] : rarity_chance_map) {
        sum += value;
        if (roll <= sum)
          return type;
      }
      return Rarity::COMMON; // default
    };

    const auto weapons_e = get_weapons(r, player_e);

    // get weapons that arnt max level (i.e. level 12)
    const int WEAPON_MAX_LEVEL = 12;
    std::vector<entt::entity> non_max_level_weapons;
    for (int i = 0; i < weapons_e.size(); i++) {
      const auto& wep_lvl_c = r.get<WeaponLevelComponent>(weapons_e[i]);
      const auto lv = wep_lvl_c.level;
      if (lv >= WEAPON_MAX_LEVEL)
        continue;
      non_max_level_weapons.push_back(weapons_e[i]);
    }

    // For the 1st & 2nd upgrade, roll a BULLET_X or WEAPON_X stat
    while (results_c.results.size() != 2) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)weapon_and_bullet_stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade_enum = weapon_and_bullet_stats[roll_value];
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));
      const auto [value, type] = stat_from_stat_table(rarity, upgrade_enum);

      // level up one random weapon that isnt max level
      int non_max_level_wep_size = non_max_level_weapons.size();

      //
      if (non_max_level_wep_size > 0) {
        const int rnd_wep_upg_idx = engine::rand_det_s(roll_rnd.rng, 0, non_max_level_wep_size);
        const auto wep_e = non_max_level_weapons[rnd_wep_upg_idx];

        results_c.results.emplace(UpgradeRollResult{
          .rarity = rarity,
          .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
          // WEAPON_x and BULLET_x do level weapon
          // .weapons = { weapon_e }, // note: only leveling first.
          .weapons = { wep_e },
          .level_weapons = true,
        });

      } else {
        results_c.results.emplace(UpgradeRollResult{
          .rarity = rarity,
          .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
          .weapons = {},
          .level_weapons = false,
        });
      }
    }

    // For the 3rd upgrade, roll an ACTOR_X stat.
    while (results_c.results.size() != 3) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)actor_x_stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade_enum = actor_x_stats[roll_value];
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));
      const auto [value, type] = stat_from_stat_table(rarity, upgrade_enum);

      results_c.results.emplace(UpgradeRollResult{
        .rarity = rarity,
        .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
        .weapons = {}, // ACTOR_x do not level weapon
        .level_weapons = false,
      });
    }

    r.emplace<UpgradeResultsComponent>(player_e, results_c);
  }

  SDL_Log("Generated upgrades for %i players", num_players);
};

void
populate_ui_based_on_upgrades(entt::registry& r, SINGLE_LevelUpUI& ui_c)
{
  SDL_Log("Populating upgrade ui...");
  const int max_num_players = 4;

  // reset ui
  for (int i = 0; i < max_num_players; i++) {
    auto& state_c = ui_c.ui_states[i];
    state_c.cells.clear();
    state_c.actions.clear();
    state_c.active = nullptr;

    const auto player_e = get_player_e_from_idx(r, i);
    if (player_e == entt::null)
      continue;

    const auto& modifier_c = r.get<StatModifierComponent>(player_e); // check it has one
    const auto upgrades = find<UpgradeResultsComponent>(r, player_e);
    if (upgrades.size() == 0)
      continue;
    const auto& [upg_e, upg_c] = upgrades[0];

    for (const UpgradeRollResult& res : upg_c->results) {
      Cell c;
      c.name = ""; // replaced with the upgrade name when upgrade is populated
      c.action = [upg_e, res, player_e]() {
        auto& evts_c = SINGLE_Events::instance;
        UpgradeEvent evt;
        evt.par_e = player_e;
        evt.upg_e = upg_e; // upg_e is wep_e or par_e
        evt.roll_result = res;
        evts_c.dispatcher->trigger(evt);
        evts_c.dispatcher->update();
      };
      state_c.cells.push_back(std::make_shared<Cell>(c));
    }

    create_as_vertical_layout(state_c.cells);
    state_c.active = state_c.cells[0];
  }
};

bool
is_choosing_upgrade(entt::registry& r)
{
  return r.view<UpgradeResultsComponent>().size() > 0;
};

entt::entity
get_player_e_from_idx(entt::registry& r, const int player_idx)
{
  const auto player_es = view_to_vector_of_ents<PlayerBoatComponent>(r);

  const auto player_with_idx = [&r, player_idx](const entt::entity player_e) {
    auto& player_c = r.get<PlayerComponent>(player_e);
    return player_c.idx == player_idx;
  };

  const auto player_it = std::find_if(player_es.begin(), player_es.end(), player_with_idx);
  if (player_it == player_es.end())
    return entt::null; // player not joined

  return (*player_it);
};

SINGLE_UpgradeToName
load_upgrade_names(const std::string& path)
{
  SDL_Log("loading upgrade names... %s", path.c_str());

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
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  SINGLE_UpgradeToName data = root.get<SINGLE_UpgradeToName>();

  // populate data stat_to_name_map
  for (const auto& upgrade_on_disk : data.names) {
    const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(upgrade_on_disk.stat).value();
    const UpgradeRollResult result{
      .rarity = magic_enum::enum_cast<Rarity>(upgrade_on_disk.rarity).value(),
      .stats = { Stat{ .stat = upgrade_on_disk.stat } },
    };
    data.stat_to_name_map[result] = upgrade_on_disk.name;
  }

  return data;
};

std::string
make_stat_name_pretty_name(const std::string stat)
{
  std::string result = stat;

  // remove ACTOR_
  result = str_remove_all_occurances(result, "ACTOR_");

  // convert from bold uppercase to regular.
  // e.g. "WEAPON_RANGE" to "Weapon Range"
  std::string result_lower = "";
  bool capitalize_next = true;
  for (const auto c : result) {
    if (c == '_') {
      result_lower.push_back(' '); // replace underscores
      capitalize_next = true;
    } else if (capitalize_next) {
      result_lower.push_back(std::toupper(c));
      capitalize_next = false;
    } else
      result_lower.push_back(std::tolower(c));
  }

  return result_lower;
};

} // namespace game2d