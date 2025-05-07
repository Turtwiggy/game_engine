#include "pch.hpp"

#include "ui_survive_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
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
    const auto& player_c = r.get<PlayerComponent>(player_e);

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
    const auto weapon_e = weapons_e[0]; // hmm: upgrade only the first wep

    // For the 1st & 2nd upgrade, roll a BULLET_X or WEAPON_X stat
    while (results_c.results.size() != 2) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)weapon_and_bullet_stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade_enum = weapon_and_bullet_stats[roll_value];
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade_enum));
      const auto [value, type] = stat_from_stat_table(rarity, upgrade_enum);

      results_c.results.emplace(UpgradeRollResult{
        .rarity = rarity,
        .stats = { Stat{ .stat = upgrade_str, .type = type, .value = value } },
        // WEAPON_x and BULLET_x do level weapon
        .weapons = { weapon_e }, // note: only leveling first.
        .level_weapons = true,
      });
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
aquire_action(entt::registry& r, entt::entity player_e, const UpgradeRollResult& roll)
{
  auto& evts_c = get_first_component<SINGLE_Events>(r);
  UpgradeEvent evt;
  evt.e = player_e;
  evt.roll_result = roll;
  evts_c.dispatcher->trigger(evt);
  evts_c.dispatcher->update();
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
    const auto& upgrades_c = r.get<UpgradeResultsComponent>(player_e);
    const auto& modifier_c = r.get<StatModifierComponent>(player_e); // check it has one

    for (const UpgradeRollResult& res : upgrades_c.results) {
      Cell c;
      c.name = "Aquire";
      c.action = [&, player_e]() { aquire_action(r, player_e, res); };
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
get_player_e_from_idx(entt::registry& r, int player_idx)
{
  const auto player_es = view_to_vector_of_ents<PlayerComponent>(r);

  auto player_with_idx = [&r, player_idx](const entt::entity player_e) {
    auto& player_c = r.get<PlayerComponent>(player_e);
    return player_c.idx == player_idx;
  };

  auto player_it = std::find_if(player_es.begin(), player_es.end(), player_with_idx);
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
}

} // namespace game2d