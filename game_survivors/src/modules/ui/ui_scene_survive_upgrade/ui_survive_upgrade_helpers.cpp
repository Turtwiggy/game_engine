#include "pch.hpp"

#include "ui_survive_upgrade_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/raws/raws_helpers.hpp"
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

    // For the 1st & 2nd upgrade, roll a BULLET_X or WEAPON_X stat
    while (results_c.results.size() != 2) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)weapon_and_bullet_stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade = weapon_and_bullet_stats[roll_value];

      results_c.results.emplace(UpgradeRollResult{ .rarity = rarity, .upgrade = upgrade });
    }

    // For the 3rd upgrade, roll an ACTOR_X stat.
    while (results_c.results.size() != 3) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)actor_x_stats.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      const auto rarity = get_rarity_from_roll(roll_rarity);
      const auto upgrade = actor_x_stats[roll_value];

      results_c.results.emplace(UpgradeRollResult{ .rarity = rarity, .upgrade = upgrade });
    }

    r.emplace<UpgradeResultsComponent>(player_e, results_c);
  }

  SDL_Log("Generated upgrades for %i players", num_players);
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
    const UpgradeRollResult result{
      .rarity = magic_enum::enum_cast<Rarity>(upgrade_on_disk.rarity).value(),
      .upgrade = magic_enum::enum_cast<UpgradeableStat>(upgrade_on_disk.stat).value(),
    };
    data.stat_to_name_map[result] = upgrade_on_disk.name;
  }

  return data;
}

} // namespace game2d